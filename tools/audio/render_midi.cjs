'use strict';
// Offline renderer for the committed Standard MIDI source files; no audio device is opened.
const fs = require('node:fs');
const crypto = require('node:crypto');
const fluid = require('js-synthesizer/externals/libfluidsynth-2.4.6-with-libsndfile.js');
const JSSynth = require('js-synthesizer');
const RATE = 44100;

function readMidi(data) {
  if (data.toString('ascii', 0, 4) !== 'MThd' || data.readUInt32BE(4) !== 6) throw Error('Invalid MIDI header');
  const format = data.readUInt16BE(8), count = data.readUInt16BE(10), ppqn = data.readUInt16BE(12);
  if (format !== 1 || ppqn === 0 || ppqn & 0x8000) throw Error('Expected format-1 PPQN MIDI');
  const events = [];
  let cursor = 14;
  for (let track = 0; track < count; track++) {
    if (data.toString('ascii', cursor, cursor + 4) !== 'MTrk') throw Error('Invalid MIDI track');
    const end = cursor + 8 + data.readUInt32BE(cursor + 4);
    cursor += 8;
    if (end > data.length) throw Error('Truncated MIDI track');
    let tick = 0, running = 0;
    const variable = () => {
      let value = 0;
      for (let n = 0; n < 4; n++) {
        if (cursor >= end) throw Error('Truncated MIDI delta');
        const byte = data[cursor++]; value = value * 128 + (byte & 127);
        if (!(byte & 128)) return value;
      }
      throw Error('Oversized MIDI delta');
    };
    while (cursor < end) {
      tick += variable();
      let status = data[cursor++];
      if (status < 128) { cursor--; status = running; }
      if (status === 0xff) {
        const type = data[cursor++], length = variable();
        if (cursor + length > end) throw Error('Truncated MIDI meta event');
        if (type === 0x51) {
          if (length !== 3) throw Error('Invalid MIDI tempo');
          events.push({tick, tempo: data.readUIntBE(cursor, 3), order: 0});
        }
        cursor += length;
      } else if (status === 0xf0 || status === 0xf7) {
        const length = variable(); cursor += length;
        if (cursor > end) throw Error('Truncated MIDI SysEx');
      } else {
        if (status < 0x80 || status >= 0xf0) throw Error('Invalid MIDI status');
        running = status;
        const type = status & 0xf0, channel = status & 15;
        const length = type === 0xc0 || type === 0xd0 ? 1 : 2;
        if (cursor + length > end) throw Error('Truncated MIDI channel event');
        const a = data[cursor++], b = length === 2 ? data[cursor++] : 0;
        if (a > 127 || b > 127) throw Error('Invalid MIDI channel value');
        events.push({tick, type, channel, a, b, order: type === 0x90 && b > 0 ? 2 : 1});
      }
    }
  }
  if (cursor !== data.length) throw Error('Unexpected trailing MIDI bytes');
  events.sort((a,b) => a.tick - b.tick || a.order - b.order);
  let lastTick = 0, seconds = 0, tempo = 500000;
  const notes = new Map();
  for (const event of events) {
    seconds += (event.tick - lastTick) * tempo / (ppqn * 1e6); lastTick = event.tick;
    event.sample = Math.round(seconds * RATE);
    if (event.tempo) tempo = event.tempo;
    if (event.type === 0x90 && event.b > 0) {
      const key = event.channel * 128 + event.a;
      if (notes.has(key)) throw Error('Overlapping MIDI note on one channel/key');
      notes.set(key, true);
    } else if (event.type === 0x80 || event.type === 0x90 && event.b === 0) {
      const key = event.channel * 128 + event.a;
      if (!notes.delete(key)) throw Error('MIDI note-off without note-on');
    }
  }
  if (notes.size) throw Error('MIDI contains stuck notes');
  return {events: events.filter(e => e.type), ppqn,
    tempos: events.filter(e => e.tempo).map(e => ({tick:e.tick,microseconds_per_quarter:e.tempo}))};
}

function apply(synth, event) {
  const {channel,a,b,type} = event;
  if (type === 0x90) b ? synth.midiNoteOn(channel,a,b) : synth.midiNoteOff(channel,a);
  else if (type === 0x80) synth.midiNoteOff(channel,a);
  else if (type === 0xb0) synth.midiControl(channel,a,b);
  else if (type === 0xc0) synth.midiProgramChange(channel,a);
  else throw Error('Unsupported composed MIDI event');
}

async function main() {
  const [midiPath,fontPath,outPath,durationText,peakText,mode] = process.argv.slice(2);
  const seconds = Number(durationText), targetPeak = Number(peakText), looping = mode === 'loop';
  if (!(seconds > 0 && seconds <= 90 && targetPeak > 0 && targetPeak < .9)) throw Error('Invalid render duration/gain');
  const source = fs.readFileSync(midiPath), parsed = readMidi(source), events = parsed.events;
  const frames = Math.round(seconds * RATE), channels = looping ? 2 : 1;
  if (events.some(e => e.sample > frames)) throw Error('MIDI event exceeds render duration');
  JSSynth.Synthesizer.initializeWithFluidSynthModule(fluid);
  await JSSynth.waitForReady();
  const synth = new JSSynth.Synthesizer();
  synth.init(RATE, {initialGain: .8, chorusActive: false, reverbActive: looping,
    reverbRoomSize: .38, reverbDamp: .45, reverbLevel: .18, reverbWidth: .65,
    midiBankSelect: 'gm', polyphony: 512, minNoteLength: 1});
  const font = fs.readFileSync(fontPath);
  const sfid = await synth.loadSFont(font.buffer.slice(font.byteOffset,font.byteOffset+font.byteLength));
  const presets = synth.getSFontObject(sfid);
  const sounding = new Set(events.filter(e => e.type === 0x90 && e.b > 0).map(e => e.channel));
  const instruments = [...new Set(events.filter(e => e.type === 0xc0 && e.channel !== 9 && sounding.has(e.channel)).map(e => e.a))]
    .map(num => ({program: num, name: presets.getPreset(0,num)?.name}));
  if (instruments.some(i => !i.name)) throw Error('Requested MIDI instrument is absent from SoundFont');
  const samples = Array.from({length: channels}, () => new Float32Array(frames));
  const scheduled = [];
  for (let cycle = 0; cycle < (looping ? 2 : 1); cycle++) {
    for (const event of events) scheduled.push({...event,sample: event.sample+cycle*frames});
  }
  scheduled.sort((a,b) => a.sample-b.sample || a.order-b.order);
  let cursor = 0, position = 0;
  const total = frames * (looping ? 2 : 1);
  while (position < total) {
    while (cursor < scheduled.length && scheduled[cursor].sample <= position) apply(synth,scheduled[cursor++]);
    const end = Math.min(total,position+4096,cursor < scheduled.length ? scheduled[cursor].sample : total);
    const count = end-position;
    if (count <= 0) throw Error('MIDI scheduler made no progress');
    const block = [new Float32Array(count),new Float32Array(count)]; synth.render(block);
    const outputStart = Math.max(position,looping ? frames : 0);
    if (end > outputStart) {
      const offset = outputStart-(looping ? frames : 0), from = outputStart-position;
      if (looping) {samples[0].set(block[0].subarray(from),offset);samples[1].set(block[1].subarray(from),offset);}
      else for (let i=from;i<count;i++) samples[0][offset+i-from]=(block[0][i]+block[1][i])*.5;
    }
    position = end;
  }
  synth.close();
  const edge = Math.min(Math.round(RATE * .003), Math.floor(frames / 4));
  for (const channel of samples) {
    if (looping) {
      const join=(channel[0]+channel[frames-1])*.5;
      for (let i=0;i<edge;i++) {
        const mix=i/(edge-1);
        channel[i]=join*(1-mix)+channel[i]*mix;
        channel[frames-1-i]=join*(1-mix)+channel[frames-1-i]*mix;
      }
    } else {
      const attack = Math.min(Math.round(RATE * .0015), Math.floor(frames / 4));
      const release = Math.min(Math.round(RATE * .012), Math.floor(frames / 4));
      for (let i = 0; i < attack; i++) channel[i] *= i / (attack - 1);
      for (let i = 0; i < release; i++) channel[frames - 1 - i] *= i / (release - 1);
    }
  }
  let rawPeak=0;
  for (const channel of samples) for (const value of channel) rawPeak=Math.max(rawPeak,Math.abs(value));
  if (!(rawPeak > .00001 && Number.isFinite(rawPeak))) throw Error('Silent/non-finite MIDI rendering');
  const gain=targetPeak*32767/rawPeak, pcm=Buffer.alloc(frames*channels*2);
  let at=0, energy=0, peak=0;
  for (let i=0;i<frames;i++) for (let ch=0;ch<channels;ch++) {
    const value=Math.round(samples[ch][i]*gain);
    if (!Number.isFinite(value) || Math.abs(value)>32767) throw Error('Clipped MIDI render');
    pcm.writeInt16LE(value,at);at+=2;peak=Math.max(peak,Math.abs(value));energy+=(value/32768)**2;
  }
  const header=Buffer.alloc(44);header.write('RIFF');header.writeUInt32LE(36+pcm.length,4);header.write('WAVEfmt ',8);
  header.writeUInt32LE(16,16);header.writeUInt16LE(1,20);header.writeUInt16LE(channels,22);header.writeUInt32LE(RATE,24);
  header.writeUInt32LE(RATE*channels*2,28);header.writeUInt16LE(channels*2,32);header.writeUInt16LE(16,34);
  header.write('data',36);header.writeUInt32LE(pcm.length,40);
  fs.writeFileSync(outPath,Buffer.concat([header,pcm]));
  console.log(JSON.stringify({midi: midiPath.split(/[\\/]/).pop(),midi_sha256:crypto.createHash('sha256').update(source).digest('hex'),
    seconds,channels,ticks_per_quarter:parsed.ppqn,tempo_changes:parsed.tempos,note_on_count:events.filter(e=>e.type===0x90&&e.b>0).length,
    instruments,drum_notes:[...new Set(events.filter(e=>e.channel===9&&e.type===0x90&&e.b>0).map(e=>e.a))].sort((a,b)=>a-b),peak:peak/32768,rms:Math.sqrt(energy/(frames*channels)),warmup_cycles:looping?1:0,
    sha256:crypto.createHash('sha256').update(fs.readFileSync(outPath)).digest('hex')}));
}
main().catch(error=>{console.error(error.stack||String(error));process.exitCode=1;});
