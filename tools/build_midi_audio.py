#!/usr/bin/env python3
"""Original General MIDI scores rendered with pinned FluidSynth/SoundFont instruments."""
import argparse
import hashlib
import json
import shutil
from pathlib import Path
import struct
import subprocess

from compose_game_music import FORMS, KEYS, TEMPOS, compose_bgm

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "art_sources/audio"
MIDI_DIR = SOURCE / "midi"
RATE = 44100
TPQ = 480
BGM = [("BGM_MENU", "menu", 32), ("BGM_STAGE01", "stage01_forest", 64),
       ("BGM_STAGE02", "stage02_lake", 64), ("BGM_STAGE03", "stage03_shrine", 64),
       ("BGM_BOSS", "boss", 64), ("BGM_ENDING", "ending", 32)]
SFX = [("SFX_SHOT", "player_shot", .10), ("SFX_ENEMY_HIT", "enemy_hit", .12),
       ("SFX_PLAYER_HIT", "player_hit", .32), ("SFX_EXPLOSION", "explosion", .45),
       ("SFX_BOMB", "bomb", .70), ("SFX_MENU_MOVE", "menu_move", .08),
       ("SFX_MENU_CONFIRM", "menu_confirm", .18), ("SFX_CLEAR", "stage_clear", 2.0),
       ("SFX_GAME_OVER", "game_over", 2.0)]
PEAKS = {"menu": .43, "stage01_forest": .50, "stage02_lake": .47, "stage03_shrine": .50,
         "boss": .53, "ending": .43, "player_shot": .09, "enemy_hit": .26,
         "player_hit": .43, "explosion": .45, "bomb": .60,
         "menu_move": .13, "menu_confirm": .23, "stage_clear": .50, "game_over": .38}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def vlq(value):
    if value < 0 or value > 0x0fffffff:
        raise ValueError("MIDI delta time out of range")
    result = [value & 127]
    while value >> 7:
        value >>= 7
        result.insert(0, (value & 127) | 128)
    return bytes(result)


def score(name, seconds, bgm):
    if bgm:
        return compose_bgm(name, seconds)
    tracks = {}

    def channel(ch, program, volume=90):
        if ch not in tracks:
            tracks[ch] = [(0, bytes([0xC0 | ch, program])), (0, bytes([0xB0 | ch, 7, volume])),
                          (0, bytes([0xB0 | ch, 10, 64])), (0, bytes([0xB0 | ch, 91, 0]))]

    def note(ch, start, length, key, velocity):
        end = min(seconds, start + length)
        if not (0 <= start < end and 0 <= key <= 127 and 1 <= velocity <= 127):
            raise ValueError("Invalid effect note")
        ticks_per_second = TPQ * 2
        tracks[ch].extend([(round(start * ticks_per_second), bytes([0x90 | ch, key, velocity])),
                           (round(end * ticks_per_second), bytes([0x80 | ch, key, 0]))])

    def drum(at, key, velocity, length=.04):
        channel(9, 0, 90)
        note(9, at, length, key, velocity)

    # Brief noise/percussion transients keep repeated combat effects unpitched.
    if name == "player_shot":
        drum(0, 42, 48, .025)
    elif name == "enemy_hit":
        drum(0, 37, 82, .035)
        drum(.006, 42, 27, .025)
    elif name == "player_hit":
        drum(0, 40, 95, .08)
        drum(.018, 43, 75, .11)
    elif name == "explosion":
        drum(0, 35, 94, .13)
        drum(.012, 40, 66, .09)
        drum(.034, 41, 81, .15)
        drum(.028, 49, 29, .18)
    elif name == "bomb":
        channel(0, 116, 85)
        note(0, 0, .45, 36, 88)
        drum(0, 35, 111, .19)
        drum(.015, 52, 60, .30)
        drum(.065, 41, 80, .23)
    elif name == "menu_move":
        channel(0, 11, 72)
        note(0, 0, .035, 79, 52)
    elif name == "menu_confirm":
        channel(0, 11, 78)
        note(0, 0, .065, 79, 61)
        note(0, .075, .065, 84, 58)
    elif name in ("stage_clear", "game_over"):
        channel(0, 0, 88)
        channel(1, 48, 42)
        if name == "stage_clear":
            melody = [(0, 72, .15), (.25, 77, .18), (.5, 81, .16), (.75, 79, .12), (.95, 84, .18), (1.2, 77, .65)]
            chord = (53, 57, 60)
        else:
            melody = [(0, 69, .28), (.35, 67, .28), (.72, 64, .34), (1.17, 62, .65)]
            chord = (50, 53, 57)
        for at, key, length in melody:
            note(0, at, length, key, 75 if name == "stage_clear" else 66)
        for key in chord:
            note(1, .01, 1.65, key, 45)
    else:
        raise ValueError("Unknown sound effect: " + name)
    return tracks



def write_midi(path, tracks, seconds, bpm=120):
    if not 30 <= bpm <= 300:
        raise ValueError('Invalid MIDI tempo')
    tempo = round(60_000_000 / bpm)
    last_tick = round(seconds * TPQ * bpm / 60)
    conductor = b'\0\xff\x51\x03' + tempo.to_bytes(3, 'big') + b'\0\xff\x58\x04\x04\x02\x18\x08'
    conductor += vlq(last_tick) + b'\xff\x2f\0'
    chunks = [conductor]
    for ch, events in sorted(tracks.items()):
        data, previous = bytearray(), 0
        name = ("Drums" if ch == 9 else f"Part {ch + 1}").encode()
        data.extend(b'\0\xff\x03' + vlq(len(name)) + name)
        # Note offs at a shared tick precede the next note on.
        for tick, event in sorted(events, key=lambda e: (e[0], e[1][0] & 0xf0 == 0x90)):
            data.extend(vlq(tick - previous) + event); previous = tick
        data.extend(vlq(last_tick - previous) + b'\xff\x2f\0')
        chunks.append(bytes(data))
    payload = b'MThd' + struct.pack('>IHHH', 6, 1, len(chunks), TPQ)
    payload += b''.join(b'MTrk' + struct.pack('>I', len(c)) + c for c in chunks)
    path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(payload)


def ensure_renderer():
    if not (ROOT / 'tools/audio/node_modules/js-synthesizer/package.json').is_file():
        raise RuntimeError('MIDI renderer missing: run npm ci --ignore-scripts --prefix tools/audio')
    font = SOURCE / 'MS-Basic.sf3'
    if not font.is_file():
        raise RuntimeError('Missing bundled art_sources/audio/MS-Basic.sf3')
    return font


def build_audio(asset_id, name, seconds, output, *, regenerate=False):
    bgm = asset_id.startswith('BGM_')
    path = MIDI_DIR / (name + '.mid')
    if regenerate or not path.is_file():
        write_midi(path, score(name, seconds, bgm), seconds, TEMPOS[name] if bgm else 120)
    font = ensure_renderer()
    output = Path(output); output.parent.mkdir(parents=True, exist_ok=True)
    command = ['node', str(ROOT / 'tools/audio/render_midi.cjs'), str(path), str(font),
               str(output), str(seconds), str(PEAKS.get(name, .52)), 'loop' if bgm else 'effect']
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(result.stderr[-2000:] or result.stdout[-2000:])
    return json.loads(result.stdout.strip().splitlines()[-1])


def build_all(resources, *, regenerate=False):
    report = {}
    for asset_id, name, seconds in BGM + SFX:
        output = Path(resources) / 'audio' / ('bgm' if asset_id.startswith('BGM_') else 'sfx') / (name + '.wav')
        report[asset_id] = build_audio(asset_id, name, seconds, output, regenerate=regenerate)
        print(f'Rendered {asset_id}', flush=True)
    license_copy = Path(resources) / 'licenses/audio/MS-Basic-LICENSE.md'
    license_copy.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(SOURCE / 'MS-Basic-LICENSE.md', license_copy)
    provenance = {
        'format': 'Standard MIDI File format 1; PPQN and tempo events are recorded per source track',
        'initial_recipe': {'ppqn': TPQ, 'tempos_bpm': TEMPOS, 'keys': KEYS, 'forms': FORMS,
                           'score_sha256': digest(ROOT / 'tools/compose_game_music.py')},
        'composition': 'Original authored themes and phrase arrangements in tools/compose_game_music.py; existing MIDI files are authoritative',
        'renderer': 'js-synthesizer 1.13.0 / FluidSynth 2.4.6 WebAssembly with libsndfile',
        'renderer_install': 'npm ci --ignore-scripts --prefix tools/audio',
        "soundfont": {"path": "art_sources/audio/MS-Basic.sf3", "sha256": digest(SOURCE / "MS-Basic.sf3"),
                      "name": "MS Basic / MuseScore_General_HQ v0.2", "license_type": "MIT",
                      "author": "MuseScore contributors; full notices in MS-Basic-LICENSE.md",
                      "url": "https://github.com/musescore/MuseScore",
                      "commit": "37128c5ac8f9af804b63650610952414421828b8",
                      "license": "art_sources/audio/MS-Basic-LICENSE.md",
                      "runtime_notice": "src/RebornBoss/resources/licenses/audio/MS-Basic-LICENSE.md"},
        'loop_processing': 'One full MIDI cycle warms sample/reverb tails; capture the next cycle and match endpoints over 3 ms.',
        'tracks': report, 'windows_verified': False,
    }
    (SOURCE / 'manifest.json').write_text(json.dumps(provenance, ensure_ascii=False, indent=2) + '\n')
    return provenance


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--regenerate-midi', action='store_true',
                        help='overwrite existing MIDI sources with the original Python score recipe')
    args = parser.parse_args()
    build_all(ROOT / 'src/RebornBoss/resources', regenerate=args.regenerate_midi)
