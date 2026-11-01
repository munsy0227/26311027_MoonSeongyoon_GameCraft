# RebornBoss MIDI soundtrack

Six original music themes and nine effects are authored for this game's fast, melodic bullet-hell style. Editable Standard MIDI files live in `midi/`. Existing MIDI files are preserved by a normal rebuild.

## SoundFont and redistribution

`MS-Basic.sf3` is the unchanged **MS Basic / MuseScore_General_HQ v0.2** bank from the official MuseScore repository at commit `37128c5ac8f9af804b63650610952414421828b8`. It is 51,278,610 bytes (48.9 MiB), below GitHub's 100 MiB regular-Git file limit. Its SHA-256 is `5ea2375e8bd7d8e71def1036978c1621e85b66934169b6a2744b27b9b3c2d99c`.

- [Official pinned SoundFont directory](https://github.com/musescore/MuseScore/tree/37128c5ac8f9af804b63650610952414421828b8/share/sound)
- [Official license](https://github.com/musescore/MuseScore/blob/37128c5ac8f9af804b63650610952414421828b8/share/sound/MS%20Basic_License.md)
- [GitHub file-size rules](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github)

The bank uses the MIT license. Keep the full copyright, acknowledgement and permission notices in `MS-Basic-LICENSE.md`; the unchanged upstream README is also included. A complete license copy is distributed in the game's `resources/licenses/audio/`. The bank itself is only needed for authoring, not game playback.

## Composition

`tools/compose_game_music.py` contains authored themes and arrangements; `tools/build_midi_audio.py` contains effect scores and the MIDI/WAV build pipeline. The previous per-chord melody generator has been replaced with explicit themes, 4/8-bar phrases, cadences and contrasting sections. The scores use original melodies; no Touhou or other game tune is copied.

| Track | BPM | Bars | Duration |
|---|---:|---:|---:|
| Menu | 150 | 20 | 32 s |
| Fairy forest | 150 | 40 | 64 s |
| Mist lake | 135 | 36 | 64 s |
| Moon shrine | 165 | 44 | 64 s |
| Boss | 180 | 48 | 64 s |
| Ending | 120 | 16 | 32 s |

Piano motion, brass/string themes, common-tone chord voicings, bass grooves and section-specific percussion provide the arrangement. Register changes are planned; notes are not folded down after generation. Combat effects use short percussion transients so repeated shots do not form a competing pitched melody. The menu intro leaves room for the short result jingle that plays with it.

Research applied to the original scores:

- [Berklee: melodic phrases, rests/resolution, stepwise motion and deliberate leaps](https://online.berklee.edu/takenote/conjunct-disjunct-melody-basic-definitions/)
- [MIT OpenCourseWare: phrase and cadence definitions](https://ocw.mit.edu/courses/21m-011-introduction-to-western-music-fall-2024/pages/week-1-basic-musical-terms-definitions/)

## Rebuild

Install Node.js and the existing Python/Pillow asset environment, then run from the project root:

```sh
npm ci --ignore-scripts --prefix tools/audio
python3 tools/build_midi_audio.py
python3 tools/build_week2_assets.py --check
```

Normal builds render existing `.mid` files and generate only missing sources. To deliberately replace MIDI edits with the authored Python scores, use:

```sh
python3 tools/build_midi_audio.py --regenerate-midi
```

Run the complete `tools/build_week2_assets.py` exporter after score changes to refresh its catalog, or update its `make_catalog`/`validate` outputs. The `--check` mode only validates files.

The pinned `js-synthesizer 1.13.0` package supplies FluidSynth 2.4.6 WebAssembly with libsndfile, required for the compressed SF3 bank. The game plays rendered 44.1 kHz/16-bit PCM WAV: stereo music and mono effects. Music warms one cycle before capture and joins the loop endpoints over 3 ms. Effects preserve their transient with 1.5 ms attack and 12 ms release protection.

`manifest.json` records actual MIDI tempo/program/note data, hashes and renderer settings. Tests cover source preservation, note pairing, PCM levels, silent effect edges and music loop boundaries. These checks do not establish subjective musical quality or Windows game playback.

js-synthesizer is BSD-3-Clause; FluidSynth is LGPL-2.1. They are npm-installed authoring tools; see the installed notices and `tools/audio/package-lock.json`.
