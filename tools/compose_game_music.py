"""Original, phrase-authored ROMpler game scores; timing is in quarter-note beats."""
from itertools import product

TPQ = 480
TEMPOS = {"menu": 150, "stage01_forest": 150, "stage02_lake": 135,
          "stage03_shrine": 165, "boss": 180, "ending": 120}
FORMS = {
    "menu": [("intro", 4), ("A", 8), ("B", 8)],
    "stage01_forest": [("intro", 4), ("A", 8), ("A_prime", 8), ("B", 8), ("A_return", 8), ("turn", 4)],
    "stage02_lake": [("intro", 4), ("A", 8), ("A_prime", 8), ("B", 8), ("A_return", 8)],
    "stage03_shrine": [("intro", 4), ("A", 8), ("A_prime", 8), ("B", 8), ("A_return", 8), ("coda", 8)],
    "boss": [("intro", 4), ("A", 8), ("A_prime", 8), ("B", 8), ("C", 8), ("A_return", 8), ("turn", 4)],
    "ending": [("A", 8), ("B", 8)],
}
KEYS = {"menu": "D minor", "stage01_forest": "F minor", "stage02_lake": "D minor",
        "stage03_shrine": "A minor", "boss": "C minor", "ending": "D major"}

# Each row is a complete bar. None is a rest; fourth/eighth bars answer the motif.
MENU_A = [
    [(69,.5),(74,1.5),(77,.5),(76,.5),(74,.5),(69,.5)],
    [(70,1),(74,.5),(77,.5),(81,1),(77,.5),(74,.5)],
    [(72,.5),(77,.5),(81,1),(79,.5),(77,.5),(76,.5),(72,.5)],
    [(76,1),(74,.5),(73,.5),(69,1.5),(None,.5)],
    [(74,.5),(79,.5),(82,.5),(81,.5),(79,1),(77,.5),(74,.5)],
    [(77,.75),(76,.25),(74,1),(72,.5),(69,.5),(65,1)],
    [(76,.5),(79,.5),(82,.5),(79,.5),(73,.5),(76,.5),(79,.5),(76,.5)],
    [(74,1.5),(69,.5),(65,1),(None,1)],
]
MENU_B = [
    [(69,.5),(72,.5),(77,1),(79,.5),(81,.5),(79,.5),(77,.5)],
    [(77,.5),(81,.5),(82,1),(81,.5),(77,.5),(74,.5),(70,.5)],
    [(72,.5),(76,.5),(79,1),(77,.5),(76,.5),(74,.5),(72,.5)],
    [(69,.5),(73,.5),(76,1),(81,.5),(79,.5),(76,.5),(73,.5)],
    [(74,.5),(77,.5),(79,.5),(81,.5),(79,1),(76,.5),(74,.5)],
    [(72,.5),(69,.5),(65,1),(69,.5),(74,.5),(72,.5),(69,.5)],
    [(76,.5),(79,.5),(82,.5),(81,.5),(79,.5),(76,.5),(73,.5),(76,.5)],
    [(73,.5),(76,.5),(79,1),(76,.5),(73,.5),(69,.5),(None,.5)],
]
FOREST_A = [
    [(72,.5),(77,.5),(80,.5),(79,.5),(77,1),(75,.5),(72,.5)],
    [(77,.75),(80,.25),(84,.5),(80,.5),(77,.5),(75,.5),(73,.5),(72,.5)],
    [(75,.5),(80,.5),(82,.5),(84,.5),(82,.5),(80,.5),(75,.5),(72,.5)],
    [(67,.5),(72,.5),(76,1),(79,.5),(77,.5),(76,.5),(72,.5)],
    [(73,.5),(77,.5),(80,.5),(82,.5),(80,1),(77,.5),(73,.5)],
    [(72,.5),(77,.5),(80,.5),(79,.5),(77,.5),(75,.5),(72,.5),(68,.5)],
    [(70,.5),(73,.5),(77,.5),(79,.5),(79,.5),(76,.5),(74,.5),(72,.5)],
    [(77,1),(72,.5),(68,.5),(65,1.5),(None,.5)],
]
FOREST_B = [
    [(68,.5),(73,.5),(77,.5),(80,.5),(79,.5),(77,.5),(75,.5),(73,.5)],
    [(70,.5),(75,.5),(79,.5),(82,.5),(80,.5),(79,.5),(77,.5),(75,.5)],
    [(72,1),(75,.5),(80,.5),(84,1),(82,.5),(80,.5)],
    [(80,.5),(79,.5),(77,.5),(75,.5),(72,1),(None,1)],
    [(77,.5),(82,.5),(85,.5),(84,.5),(82,1),(80,.5),(77,.5)],
    [(80,1),(77,.5),(75,.5),(73,1),(72,.5),(68,.5)],
    [(79,.25),(77,.25),(73,.5),(70,.5),(67,.5),(72,.5),(76,.5),(79,.5),(76,.5)],
    [(79,.5),(76,.5),(74,.5),(72,.5),(70,1),(76,.5),(None,.5)],
]
LAKE_A = [
    [(None,.5),(74,.75),(77,.25),(81,.5),(79,.5),(77,.5),(76,.5),(74,.5)],
    [(72,1.5),(74,.5),(77,1),(76,.5),(72,.5)],
    [(70,.5),(74,.5),(79,1),(77,.5),(76,.5),(74,.5),(70,.5)],
    [(73,.75),(76,.25),(79,.5),(76,.5),(73,1),(69,.5),(None,.5)],
    [(77,.5),(76,.5),(74,1),(69,.5),(72,.5),(74,.5),(77,.5)],
    [(79,.75),(77,.25),(76,.5),(74,.5),(72,1),(69,.5),(67,.5)],
    [(76,.5),(79,.5),(82,.5),(79,.5),(76,.5),(73,.5),(70,.5),(69,.5)],
    [(74,2),(77,.5),(76,.5),(74,.5),(None,.5)],
]
LAKE_B = [
    [(77,.5),(81,.5),(84,1),(81,.5),(79,.5),(77,.5),(76,.5)],
    [(79,.75),(82,.25),(81,.5),(79,.5),(77,1),(74,.5),(70,.5)],
    [(76,.5),(79,.5),(84,1),(82,.5),(79,.5),(76,.5),(72,.5)],
    [(73,1),(76,.5),(81,.5),(79,1),(None,1)],
    [(79,.5),(77,.5),(74,.75),(70,.25),(74,.5),(77,.5),(79,.5),(82,.5)],
    [(81,1),(77,.5),(76,.5),(74,1),(72,.5),(69,.5)],
    [(76,.5),(74,.25),(72,.25),(70,.5),(67,.5),(73,.5),(76,.5),(79,.5),(81,.5)],
    [(79,.5),(76,.5),(73,1),(69,1.5),(None,.5)],
]
SHRINE_A = [
    [(76,.25),(81,.25),(84,.5),(83,.5),(81,.5),(79,.5),(76,.5),(72,.5),(76,.5)],
    [(81,.5),(79,.25),(77,.25),(76,.5),(72,.5),(77,1),(81,.5),(79,.5)],
    [(77,.5),(81,.5),(84,.5),(83,.5),(81,.5),(79,.5),(77,.5),(74,.5)],
    [(71,.5),(76,.5),(80,1),(83,.5),(81,.5),(80,.5),(76,.5)],
    [(79,.5),(84,.5),(83,.5),(81,.5),(79,1),(76,.5),(72,.5)],
    [(77,.25),(81,.25),(84,.5),(81,.5),(79,.5),(77,.5),(76,.5),(74,.5),(72,.5)],
    [(71,.5),(74,.5),(77,.5),(81,.5),(83,.5),(80,.5),(78,.5),(76,.5)],
    [(81,1),(76,.5),(72,.5),(69,1.5),(None,.5)],
]
SHRINE_B = [
    [(77,.5),(81,.5),(84,1),(83,.5),(81,.5),(79,.5),(77,.5)],
    [(76,.5),(79,.5),(83,.75),(84,.25),(83,.5),(79,.5),(76,.5),(72,.5)],
    [(74,.5),(77,.5),(81,.5),(84,.5),(86,.5),(84,.5),(81,.5),(77,.5)],
    [(80,1),(83,.5),(81,.5),(80,1),(None,1)],
    [(81,.25),(79,.25),(77,.5),(76,.5),(77,.5),(81,1),(84,.5),(81,.5)],
    [(79,.5),(76,.5),(72,.75),(71,.25),(72,.5),(76,.5),(79,.5),(83,.5)],
    [(81,.5),(77,.5),(74,.5),(71,.5),(76,.5),(80,.5),(83,.5),(80,.5)],
    [(83,.5),(80,.5),(78,.5),(76,.5),(71,1),(80,.5),(None,.5)],
]
BOSS_A = [
    [(72,.5),(79,.5),(75,.25),(74,.25),(72,.5),(70,.5),(67,.5),(70,.5),(72,.5)],
    [(75,.5),(80,.5),(84,.5),(82,.5),(80,.5),(79,.5),(75,.5),(72,.5)],
    [(74,.25),(75,.25),(77,.5),(82,.5),(79,.5),(77,1),(75,.5),(74,.5)],
    [(67,.5),(71,.5),(74,1),(77,.5),(75,.5),(74,.5),(71,.5)],
    [(68,.5),(72,.5),(77,.5),(79,.5),(80,1),(79,.5),(77,.5)],
    [(79,.5),(75,.5),(72,1),(70,.5),(67,.5),(63,.5),(67,.5)],
    [(65,.5),(68,.5),(72,.5),(74,.5),(74,.5),(71,.5),(69,.5),(67,.5)],
    [(72,.5),(75,.5),(79,1),(84,1),(None,.5),(79,.5)],
]
BOSS_B = [
    [(75,.5),(79,.5),(82,.5),(84,.5),(82,1),(77,.5),(75,.5)],
    [(80,.5),(84,.5),(82,.5),(80,.5),(79,1),(75,.5),(72,.5)],
    [(77,.5),(82,.5),(86,.5),(84,.5),(82,.5),(80,.5),(77,.5),(74,.5)],
    [(79,.5),(83,.5),(86,1),(83,.5),(79,.5),(77,.5),(74,.5)],
    [(80,.5),(79,.5),(77,1),(72,.5),(75,.5),(77,.5),(80,.5)],
    [(84,.75),(82,.25),(80,.5),(79,.5),(75,1),(72,.5),(68,.5)],
    [(77,.5),(74,.5),(72,.25),(68,.25),(65,.5),(67,.5),(71,.5),(74,.5),(77,.5)],
    [(74,.5),(71,.5),(67,1),(62,1),(None,1)],
]
BOSS_C = [
    [(72,.25),(72,.25),(75,.5),(79,.5),(75,.5),(74,.25),(72,.25),(70,.5),(67,.5),(72,.5)],
    [(70,.5),(75,.5),(79,.5),(82,.5),(79,1),(75,.5),(70,.5)],
    [(68,.25),(72,.25),(75,.5),(80,1),(79,.5),(75,.5),(72,.5),(68,.5)],
    [(71,.5),(74,.5),(77,1),(79,.5),(77,.5),(74,.5),(71,.5)],
    [(77,.25),(77,.25),(80,.5),(84,.5),(80,.5),(79,.25),(77,.25),(75,.5),(72,.5),(77,.5)],
    [(72,.5),(75,.5),(80,.75),(79,.25),(75,.5),(72,.5),(68,.5),(65,.5)],
    [(74,.25),(77,.25),(80,.5),(84,.5),(80,.5),(79,.25),(77,.25),(74,.5),(71,.5),(67,.5)],
    [(71,.5),(74,.5),(79,1),(77,.5),(74,.5),(71,.5),(None,.5)],
]
ENDING_A = [
    [(69,.5),(74,1.5),(78,.5),(76,.5),(74,.5),(69,.5)],
    [(71,1),(74,.5),(78,.5),(81,1),(78,.5),(74,.5)],
    [(74,.5),(78,.5),(81,1),(79,.5),(78,.5),(76,.5),(74,.5)],
    [(76,1),(74,.5),(73,.5),(69,1.5),(None,.5)],
    [(74,.5),(79,.5),(83,.5),(81,.5),(79,1),(78,.5),(74,.5)],
    [(78,.75),(76,.25),(74,1),(73,.5),(69,.5),(66,1)],
    [(76,.5),(79,.5),(83,.5),(79,.5),(73,.5),(76,.5),(79,.5),(76,.5)],
    [(74,1.5),(69,.5),(66,1),(None,1)],
]
ENDING_B = [
    [(71,.5),(74,.5),(79,1),(81,.5),(83,.5),(81,.5),(79,.5)],
    [(69,.5),(73,.5),(76,1),(79,.5),(78,.5),(76,.5),(73,.5)],
    [(74,1),(78,.5),(81,.5),(83,1),(81,.5),(78,.5)],
    [(78,1),(76,.5),(74,.5),(71,1),(None,1)],
    [(76,.5),(79,.5),(83,1),(81,.5),(79,.5),(78,.5),(76,.5)],
    [(79,1),(78,.5),(76,.5),(74,1),(71,.5),(67,.5)],
    [(76,.5),(79,.5),(83,.5),(81,.5),(79,.5),(76,.5),(73,.5),(69,.5)],
    [(74,2),(69,.5),(66,.5),(74,.5),(None,.5)],
]

# Root pitches are near C3. The optional bass replaces the root for inversions.
CHORDS = {
    "Dm": (50,"m",38), "Dm7": (50,"m7",38), "Dm/F": (50,"m7",41), "Dm/A": (50,"m",33),
    "Bb": (58,"maj7",34), "F/A": (53,"M",33), "F": (53,"M",41),
    "C": (48,"M",36), "C7": (48,"7",36), "Gm": (55,"m7",31), "Bb7": (58,"7",34),
    "Eh": (52,"h",40), "A7": (57,"7",33), "Fm": (53,"m",41),
    "Fm/C": (53,"m7",36), "Db": (49,"maj7",37), "Ab/C": (56,"M",36),
    "Eb": (51,"7",39), "Ab": (56,"maj7",32), "Bbm": (58,"m7",34),
    "Gh": (55,"h",31), "Am": (57,"m",33), "Am/E": (57,"m7",40),
    "Fmaj": (53,"maj7",41), "E7": (52,"7",40), "C/G": (48,"maj7",31),
    "Bh": (59,"h",35), "Cm": (48,"m",36), "Cm/Eb": (48,"m7",39),
    "Cm/Bb": (48,"m7",34), "G7": (55,"7",31), "Dh": (50,"h",38),
    "D": (50,"M",38), "D6": (50,"6",38), "D/F#": (50,"maj7",42),
    "Bm": (59,"m7",35), "Gmaj": (55,"maj7",31), "Em": (52,"m7",40),
}
QUALITIES = {"M": (0,4,7), "m": (0,3,7), "7": (0,4,7,10),
             "maj7": (0,4,7,11), "m7": (0,3,7,10), "h": (0,3,6,10), "6": (0,4,7,9)}
HARMONIES = {
    "menu": ["Dm","Bb","F/A","A7","Gm","Dm/F",("Eh","A7"),"Dm"],
    "stage01_forest": ["Fm","Db","Ab/C","C7","Bbm","Fm/C",("Gh","C7"),"Fm"],
    "stage02_lake": ["Dm","Bb","Gm","A7","Dm/F","C",("Eh","A7"),"Dm"],
    "stage03_shrine": ["Am","Fmaj","Dm7","E7","C/G","Fmaj",("Bh","E7"),"Am"],
    "boss": ["Cm","Ab","Bb7","G7","Fm","Cm/Eb",("Dh","G7"),"Cm"],
    "ending": ["D","Bm","D/F#","A7","Gmaj","D/F#",("Em","A7"),"D6"],
}
BRIDGES = {
    "menu": ["Fmaj","Bb","C","A7","Gm","Dm/F",("Eh","A7"),"A7"],
    "stage01_forest": ["Db","Eb","Ab/C","Fm","Bbm","Db",("Gh","C7"),"C7"],
    "stage02_lake": ["Fmaj","Bb","C","A7","Gm","Dm/F",("Eh","A7"),"A7"],
    "stage03_shrine": ["Fmaj","C/G","Dm7","E7","Fmaj","C/G",("Bh","E7"),"E7"],
    "boss": ["Eb","Ab","Bb7","G7","Fm","Ab/C",("Dh","G7"),"G7"],
    "ending": ["Gmaj","A7","D","Bm","Em","Gmaj",("Em","A7"),"D6"],
}
PROFILES = {
    "menu": (60,0,48,33,71), "stage01_forest": (56,0,48,33,73),
    "stage02_lake": (68,4,48,33,27), "stage03_shrine": (56,107,48,34,77),
    "boss": (56,0,48,34,29), "ending": (0,24,48,32,73),
}
THEMES = {"menu": (MENU_A,MENU_B), "stage01_forest": (FOREST_A,FOREST_B),
          "stage02_lake": (LAKE_A,LAKE_B), "stage03_shrine": (SHRINE_A,SHRINE_B),
          "boss": (BOSS_A,BOSS_B), "ending": (ENDING_A,ENDING_B)}


def _voicing(name, previous):
    root, quality, _ = CHORDS[name]
    base = [root + interval for interval in QUALITIES[quality]]
    candidates = []
    for inversion, shift in product(range(len(base)), (-12,0,12)):
        notes = sorted([p + (12 if i < inversion else 0) + shift for i,p in enumerate(base)])
        if 48 <= notes[0] and notes[-1] <= 76:
            cost = sum(min(abs(p-q) for q in previous) for p in notes)
            cost += sum(min(abs(p-q) for p in notes) for q in previous)
            candidates.append((cost, abs(sum(notes)/len(notes)-62), notes))
    return min(candidates)[2]


def _bar_theme(name, section, local):
    a,b = THEMES[name]
    melody = list((b if section == "B" else BOSS_C if section == "C" else a)[local % 8])
    if section == "A_prime" and local % 8 == 6:
        # Written cadence ornament; the rest of the recognisable phrase stays intact.
        if name == "stage01_forest":
            melody = [(67,.25),(70,.25),(73,.25),(77,.25),(79,1),(76,.5),(74,.5),(72,.5),(76,.5)]
        elif name == "boss":
            melody = [(62,.25),(65,.25),(68,.25),(72,.25),(74,1),(71,.5),(69,.5),(67,.5),(71,.5)]
    if section == "A_return" and local % 8 == 1 and name == "stage01_forest":
        melody = [(77,.5),(80,.5),(85,.5),(84,.5),(80,.5),(77,.5),(75,.5),(73,.5)]
    if section in {"A_return","coda"} and local % 8 == 7:
        melody = {
            "stage01_forest": [(79,.5),(76,.5),(74,.5),(72,.5),(70,1),(76,.5),(None,.5)],
            "stage02_lake": [(76,.5),(73,.5),(69,1),(67,1.5),(None,.5)],
            "stage03_shrine": [(83,.5),(80,.5),(78,.5),(76,.5),(71,1),(80,.5),(None,.5)],
            "boss": [(86,.5),(83,.5),(79,1),(77,.5),(74,.5),(71,.5),(None,.5)],
        }[name]
    return melody


def compose_bgm(name, seconds):
    """Return MIDI channel events; the caller writes conductor tempo and track ends."""
    if name not in TEMPOS:
        raise ValueError("Unknown music score: " + name)
    total_bars = sum(count for _,count in FORMS[name])
    if abs(total_bars * 4 * 60 / TEMPOS[name] - seconds) > 0.001:
        raise ValueError("Score duration does not match its authored form")
    tracks, pending = {}, {}
    programs = PROFILES[name]
    volumes = (84,61,39,76,49)
    pans = (64,44,78,64,86)
    for ch,program in enumerate(programs):
        tracks[ch] = [(0,bytes([0xC0|ch,program])), (0,bytes([0xB0|ch,7,volumes[ch]])),
                      (0,bytes([0xB0|ch,10,pans[ch]])), (0,bytes([0xB0|ch,91,28]))]
    tracks[9] = [(0,bytes([0xC9,0])), (0,bytes([0xB9,7,60])), (0,bytes([0xB9,10,64]))]
    last_off = {}

    def note(ch, start, length, pitch, velocity, gate=.88):
        if pitch is None:
            return
        on = round(start*TPQ)
        off = min(total_bars*4*TPQ-1, round((start+length*gate)*TPQ))
        if not 0 <= pitch <= 127 or not 0 < velocity <= 127 or off <= on:
            raise ValueError("Invalid authored music note")
        if on < last_off.get((ch,pitch),-1):
            raise ValueError(f"Overlapping note on channel {ch}, pitch {pitch}")
        tracks[ch].extend([(on,bytes([0x90|ch,pitch,velocity])),(off,bytes([0x80|ch,pitch,0]))])
        last_off[ch,pitch] = off

    def strings(start, length, voicing, velocity):
        wanted = set(voicing)
        for pitch in list(pending):
            began, end, vel = pending[pitch]
            if pitch not in wanted or start > end + .01:
                note(2,began,end-began,pitch,vel,gate=.995)
                del pending[pitch]
        for pitch in wanted:
            if pitch in pending:
                began, _, vel = pending[pitch]
                pending[pitch] = began, start+length, vel
            else:
                pending[pitch] = start, start+length, velocity

    previous = [55,60,64]
    bar_index = 0
    for section, count in FORMS[name]:
        energy = 0 if section == "intro" else 1 if section == "A" else 2 if section in {"B","A_prime","coda"} else 3
        for local in range(count):
            begin = bar_index*4
            chord = (BRIDGES[name] if section == "B" else HARMONIES[name])[local % 8]
            if section == "C":
                chord = ["Cm","Cm/Bb","Ab","G7","Fm","Ab/C",("Dh","G7"),"G7"][local % 8]
            if section == "intro":
                chord = [HARMONIES[name][0],HARMONIES[name][1],HARMONIES[name][4],
                         "C7" if name=="stage01_forest" else "E7" if name=="stage03_shrine" else "G7" if name=="boss" else "A7"][local % 4]
            if section == "turn":
                chord = HARMONIES[name][local+4]
                if local==count-1:
                    chord = "C7" if name=="stage01_forest" else "G7"
            if section in {"coda","A_return"} and local == count-1 and name != "ending":
                chord = "C7" if name=="stage01_forest" else "E7" if name=="stage03_shrine" else "G7" if name=="boss" else "A7"
            spans = [(chord,0,4)] if isinstance(chord,str) else [(chord[0],0,2),(chord[1],2,2)]
            voices = []
            for chord_name, at, length in spans:
                voicing = _voicing(chord_name, previous)
                previous = voicing
                voices.append((at,length,voicing,chord_name))
                if not (name=="menu" and section=="intro" and local<2):
                    strings(begin+at,length,voicing,40+energy*3)
                bass = CHORDS[chord_name][2]
                pulses = [0,2] if name in {"menu","ending","stage02_lake"} else [0,1,2,3]
                for beat in pulses:
                    if at <= beat < at+length:
                        pitch = bass + (7 if beat in (1,3) else 0)
                        note(3,begin+beat,.8,pitch,64+energy*3)
                dense = name in {"boss","stage03_shrine"} and energy>=1 or name=="stage01_forest" and energy>=2
                step = .25 if dense else .5 if energy>0 else 1
                # Piano/harp changes density with form; the bridge opens space again.
                if section=="B": step=.5
                pattern = [0,1,2,1,3,2,1,2] if dense else [0,2,1,2]
                for n in range(round(length/step)):
                    when = at+n*step
                    pitch = voicing[pattern[n%len(pattern)] % len(voicing)]
                    if dense and n%4==2 and pitch+12<=84: pitch+=12
                    note(1,begin+when,step*.82,pitch,44+energy*3+(4 if n%4==0 else 0))
            if section == "intro":
                melody = [(None,4)]
                if name != "menu" and local==count-1:
                    root = CHORDS[spans[-1][0]][0]+12
                    if name in {"stage01_forest","stage03_shrine"}: root += 12
                    melody = [(None,3),(root,.5),(root+4,.5)]
            elif section == "turn":
                melody = list(THEMES[name][0][local+4])
                if local==count-1:
                    dominant = 72 if name=="stage01_forest" else 76 if name=="stage03_shrine" else 67 if name=="boss" else 69
                    melody = [(dominant,1),(dominant+4,.5),(dominant+7,.5),(dominant+4,1),(None,1)]
            else:
                melody = _bar_theme(name,section,local)
            if abs(sum(length for _,length in melody)-4)>1e-9:
                raise ValueError(f"Incomplete melody bar {name}/{section}/{local}")
            beat = 0
            for index,(pitch,length) in enumerate(melody):
                if pitch is not None:
                    if not 60<=pitch<=86:
                        raise ValueError(f"Unplanned lead register: {name}: {pitch}")
                    velocity = (69 if name=="ending" else 72)+energy*3
                    if beat in (0,2): velocity += 4
                    if local%4==3: velocity -= 4
                    note(0,begin+beat,length,pitch,velocity,gate=.92 if length>=1 else .85)
                elif section != "intro" and energy>=2 and length>=.5:
                    # Answer only when the lead breathes; never double its entire melody.
                    voice = voices[-1][2]
                    answers = sorted({p+octave for p in voice for octave in (0,12) if 64<=p+octave<=84})
                    answer = answers[-1]
                    note(4,begin+beat,length*.45,answer,55)
                    if length>=1:note(4,begin+beat+length*.5,length*.4,answers[-2],49)
                beat += length
            if name not in {"menu","ending"} and section != "intro":
                half_time = name=="stage02_lake"
                kicks = (0,2.5) if half_time else (0,1.5,2.5)
                snares = (2,) if half_time else (1,3)
                for beat in kicks:note(9,begin+beat,.12,36,56+energy*3)
                for beat in snares:note(9,begin+beat,.12,38,48+energy*3)
                for step in range(8):note(9,begin+step*.5,.08,42,28+(6 if step%2==0 else 0))
                if local==count-1:
                    for at,pitch in ((3,45),(3.5,47),(3.75,50)):
                        note(9,begin+at,.10,pitch,45+energy*2)
            bar_index += 1
    for pitch,(began,end,velocity) in pending.items():
        note(2,began,end-began,pitch,velocity,gate=.995)
    return tracks
