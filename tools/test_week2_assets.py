import ast
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import wave
import xml.etree.ElementTree as ET

from PIL import Image
from build_character_assets import check_border
from build_week2_assets import centered, export_image, inspect_wav, make_sfx, runtime_paths, ui_source, RATE, ROOT


class Week2AssetsTest(unittest.TestCase):
    def test_image_jobs_have_complete_sizes_and_sources(self):
        jobs = json.loads((ROOT / "art_sources/week2/asset_jobs.json").read_text())["jobs"]
        self.assertEqual(len(jobs), 25)
        self.assertEqual(len({j["path"] for j in jobs}), 25)
        for job in jobs:
            self.assertTrue((ROOT / "art_sources/week2/generated" / (job["name"] + ".png")).is_file())
            if job["columns"] > 1:
                self.assertEqual(job["size"], [job["cell"][0] * job["columns"], job["cell"][1]])

    def test_procedural_surfaces_keep_corners_and_export_without_cropping(self):
        jobs = json.loads((ROOT / "art_sources/week2/asset_jobs.json").read_text())["jobs"]
        for job in jobs:
            if not job.get("procedural"):
                continue
            image = ui_source(job)
            self.assertEqual(image.mode, "RGBA")
            self.assertEqual(image.size, tuple(job["size"]))
            if job["procedural"] == "surface":
                self.assertEqual(image.getpixel((0, 0))[3], 0)
                self.assertEqual(image.getpixel((16, 16)), (255, 255, 255, 255))
                self.assertEqual(image.getpixel((16, 0))[3], 255)
            if job["procedural"] == "panel":
                self.assertEqual(image.getextrema(), ((26, 26), (32, 32), (48, 48), (255, 255)))
            if job["procedural"] == "lock":
                self.assertEqual(image.size, (24, 24))
                self.assertEqual(image.getpixel((0, 0))[3], 0)
                self.assertEqual(image.getpixel((12, 11))[3], 255)
                self.assertLess(image.getpixel((12, 16))[0], image.getpixel((7, 16))[0])
            with tempfile.TemporaryDirectory() as folder:
                temporary = Path(folder)
                (temporary / "generated").mkdir()
                image.save(temporary / "generated" / (job["name"] + ".png"))
                with patch("build_week2_assets.ART", temporary), patch("build_week2_assets.RES", temporary):
                    export_image(job)
                self.assertEqual(Image.open(temporary / job["path"]).tobytes(), image.tobytes())

    def test_effect_frames_keep_relative_size_and_common_center(self):
        frames = centered([Image.new("RGBA", (20, 20), "white"),
                           Image.new("RGBA", (60, 60), "white")], (64, 64))
        for frame in frames:
            check_border(frame)
            left, top, right, bottom = frame.getchannel("A").getbbox()
            self.assertAlmostEqual((left + right) / 2, 32, delta=0.5)
            self.assertAlmostEqual((top + bottom) / 2, 32, delta=0.5)
        self.assertLess(frames[0].getchannel("A").getbbox()[2] - frames[0].getchannel("A").getbbox()[0], 30)

    def test_midi_sources_and_renderer_provenance_match_audio(self):
        from build_midi_audio import BGM, SFX, SOURCE, digest
        manifest = json.loads((SOURCE / "manifest.json").read_text())
        self.assertEqual(digest(SOURCE / "MS-Basic.sf3"), manifest["soundfont"]["sha256"])
        self.assertEqual((SOURCE / "MS-Basic-LICENSE.md").read_bytes(),
                         (ROOT / manifest["soundfont"]["runtime_notice"]).read_bytes())
        self.assertEqual(set(manifest["tracks"]), {spec[0] for spec in BGM + SFX})
        for asset_id, name, seconds in BGM + SFX:
            entry = manifest["tracks"][asset_id]
            data = (SOURCE / "midi" / (name + ".mid")).read_bytes()
            self.assertEqual(data[:4], b"MThd")
            self.assertEqual(data[8:10], b"\0\1")
            self.assertEqual(digest(SOURCE / "midi" / (name + ".mid")), entry["midi_sha256"])
            self.assertGreater(entry["note_on_count"], 0)
            self.assertEqual(entry["seconds"], seconds)
            self.assertTrue(entry["instruments"] or entry["drum_notes"])
            wav = ROOT / "src/RebornBoss/resources/audio" / ("bgm" if asset_id.startswith("BGM_") else "sfx") / (name + ".wav")
            self.assertEqual(digest(wav), entry["sha256"])
        self.assertLess(manifest["tracks"]["SFX_SHOT"]["peak"], .2)

    def test_authored_music_fits_whole_bars_and_releases_every_note(self):
        from build_midi_audio import BGM, write_midi
        from compose_game_music import FORMS, TEMPOS, TPQ, compose_bgm
        for _, name, seconds in BGM:
            bars = sum(count for _, count in FORMS[name])
            self.assertAlmostEqual(bars * 4 * 60 / TEMPOS[name], seconds)
            tracks = compose_bgm(name, seconds)
            for ch, events in tracks.items():
                sounding = set()
                for tick, event in sorted(events, key=lambda e: (e[0], e[1][0] & 0xf0 == 0x90)):
                    self.assertGreaterEqual(tick, 0)
                    self.assertLess(tick, bars * 4 * TPQ)
                    if event[0] & 0xf0 == 0x90:
                        self.assertNotIn(event[1], sounding, (name, ch, tick))
                        sounding.add(event[1])
                    elif event[0] & 0xf0 == 0x80:
                        self.assertIn(event[1], sounding, (name, ch, tick))
                        sounding.remove(event[1])
                self.assertFalse(sounding, (name, ch))
            with tempfile.TemporaryDirectory() as folder:
                midi = Path(folder) / (name + ".mid")
                write_midi(midi, tracks, seconds, TEMPOS[name])
                data = midi.read_bytes()
                tempo = data.index(b"\xff\x51\x03") + 3
                self.assertEqual(int.from_bytes(data[tempo:tempo + 3], "big"), round(60_000_000 / TEMPOS[name]))
            with self.assertRaises(ValueError):
                compose_bgm(name, seconds + 1)

    @unittest.skipUnless(shutil.which("node") and (ROOT / "tools/audio/node_modules/js-synthesizer/package.json").is_file(),
                         "install the optional MIDI build tool with npm ci --prefix tools/audio")
    def test_generated_effect_is_valid_pcm_with_silent_edges(self):
        with tempfile.TemporaryDirectory() as folder:
            with patch("build_week2_assets.RES", Path(folder)), \
                    patch("build_midi_audio.MIDI_DIR", Path(folder) / "midi"):
                make_sfx(("SFX_MENU_MOVE", "menu_move", 0.08))
            report = inspect_wav(Path(folder) / "audio/sfx/menu_move.wav", False)
        self.assertEqual(report["seconds"], 0.08)
        self.assertEqual(report["boundary_jump"], 0)

    @unittest.skipUnless(shutil.which("node") and (ROOT / "tools/audio/node_modules/js-synthesizer/package.json").is_file(),
                         "install the optional MIDI build tool with npm ci --prefix tools/audio")
    def test_midi_render_preserves_edited_source_and_reports_actual_instrument(self):
        from build_midi_audio import build_audio, digest, write_midi
        with tempfile.TemporaryDirectory() as folder:
            midi_dir = Path(folder) / "midi"
            source = midi_dir / "menu_move.mid"
            write_midi(source, {0: [(0, b"\xc0\x0b"), (0, b"\x90\x43\x50"),
                                    (48, b"\x80\x43\x00")]}, .08)
            before, modified = source.read_bytes(), source.stat().st_mtime_ns
            with patch("build_midi_audio.MIDI_DIR", midi_dir):
                report = build_audio("SFX_MENU_MOVE", "menu_move", .08, Path(folder) / "edited.wav")
                self.assertEqual(source.read_bytes(), before)
                self.assertEqual(source.stat().st_mtime_ns, modified)
                self.assertEqual(report["midi_sha256"], digest(source))
                self.assertEqual(report["instruments"], [{"program": 11, "name": "Vibraphone"}])
                build_audio("SFX_MENU_MOVE", "menu_move", .08, Path(folder) / "reset.wav", regenerate=True)
                self.assertNotEqual(source.read_bytes(), before)

    @unittest.skipUnless(shutil.which("node") and (ROOT / "tools/audio/node_modules/js-synthesizer/package.json").is_file(),
                         "install the optional MIDI build tool with npm ci --prefix tools/audio")
    def test_midi_renderer_rejects_overlapping_note_pairs(self):
        from build_midi_audio import SOURCE, write_midi
        events = {0: [(0, b"\xc0\x0c"), (0, b"\x90\x3c\x50"), (48, b"\x90\x3c\x50"),
                      (96, b"\x80\x3c\x00"), (120, b"\x80\x3c\x00")]}
        with tempfile.TemporaryDirectory() as folder:
            midi, output = Path(folder) / "overlap.mid", Path(folder) / "bad.wav"
            write_midi(midi, events, .2)
            result = subprocess.run(["node", str(ROOT / "tools/audio/render_midi.cjs"), str(midi),
                                     str(SOURCE / "MS-Basic.sf3"), str(output), ".2", ".3", "effect"],
                                    capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Overlapping MIDI note", result.stderr)
            self.assertFalse(output.exists())

    def test_invalid_sample_width_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "bad.wav"
            with wave.open(str(path), "wb") as output:
                output.setnchannels(1)
                output.setsampwidth(1)
                output.setframerate(RATE)
                output.writeframes(bytes([128]) * 4410)
            with self.assertRaises(AssertionError):
                inspect_wav(path, False)


class ProjectStructureTest(unittest.TestCase):
    def test_runtime_resources_exist_and_are_not_ignored_in_git_checkouts(self):
        paths = {"src/RebornBoss/resources/" + path for path in runtime_paths()}
        self.assertTrue(paths)
        for path in paths:
            self.assertTrue((ROOT / path).is_file(), path)
        if (ROOT / ".git").exists() and shutil.which("git"):
            result = subprocess.run(
                ["git", "check-ignore", "--no-index", "-z", "--stdin"], cwd=ROOT,
                input="\0".join(sorted(paths)) + "\0", text=True, capture_output=True
            )
            self.assertIn(result.returncode, (0, 1), result.stderr)
            ignored = set(result.stdout.split("\0")) - {""}
            self.assertFalse(ignored, sorted(ignored))

    def test_bundled_fonts_and_licenses_match_manifest(self):
        root = ROOT / "src/RebornBoss/resources/fonts"
        manifest = json.loads((root / "fonts.json").read_text())
        self.assertEqual({font["file"] for font in manifest["fonts"]}, {path.name for path in root.glob("*.ttf")})
        self.assertEqual({style for font in manifest["fonts"] for style in font["styles"]},
                         {"Title", "Heading", "Body", "Caption"})
        for font in manifest["fonts"]:
            self.assertEqual(hashlib.sha256((root / font["file"]).read_bytes()).hexdigest(), font["sha256"])
            self.assertTrue(font["licenses"])
            for name in font["licenses"]:
                self.assertEqual(hashlib.sha256((root / name).read_bytes()).hexdigest(), manifest["license_sha256"][name])
        self.assertNotIn("__MACOSX", {part for path in root.rglob("*") for part in path.parts})

    @unittest.skipUnless(shutil.which("fc-scan"), "Fontconfig is required for font character coverage inspection")
    def test_bundled_font_families_cover_game_strings(self):
        root = ROOT / "src/RebornBoss/resources/fonts"
        manifest = json.loads((root / "fonts.json").read_text())
        characters = set()
        for path in (ROOT / "src/RebornBoss").glob("*.cpp"):
            for literal in re.findall(r'"((?:\\.|[^"\\])*)"', path.read_bytes().decode("cp949")):
                characters.update(ord(char) for char in literal if ord(char) >= 32)
        for font in manifest["fonts"]:
            info = subprocess.check_output(["fc-scan", "--format", "%{family}\\n%{charset}", str(root / font["file"])], text=True)
            family, charset = info.split("\n", 1)
            self.assertIn(font["family"], family.split(","))
            ranges = [tuple(int(value, 16) for value in token.split("-")) for token in charset.split()]
            missing = {chr(char) for char in characters if not any(bounds[0] <= char <= bounds[-1] for bounds in ranges)}
            self.assertFalse(missing, (font["file"], sorted(missing)))

    def test_all_sources_registered_once_in_project_and_filters(self):
        project = ROOT / "src/RebornBoss"
        ns = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        for name in ["RebornBoss.vcxproj", "RebornBoss.vcxproj.filters"]:
            tree = ET.parse(project / name)
            for kind, extension in [("ClCompile", ".cpp"), ("ClInclude", ".h")]:
                included = [e.attrib["Include"] for e in tree.findall(f".//m:{kind}[@Include]", ns)]
                self.assertEqual(len(included), len(set(included)), name)
                self.assertEqual(set(included), {f.name for f in project.glob("*" + extension)}, name)

    def test_cpp20_copy_and_debug_directory_are_shared(self):
        ns = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        tree = ET.parse(ROOT / "src/RebornBoss/RebornBoss.vcxproj")
        shared = None
        for group in tree.findall("m:ItemDefinitionGroup", ns):
            if "Condition" not in group.attrib:
                shared = group
        self.assertIsNotNone(shared)
        self.assertEqual(shared.find("m:ClCompile/m:LanguageStandard", ns).text, "stdcpp20")
        self.assertIn("/source-charset:.949", shared.find("m:ClCompile/m:AdditionalOptions", ns).text)
        self.assertIn("NOMINMAX", shared.find("m:ClCompile/m:PreprocessorDefinitions", ns).text)
        self.assertEqual(tree.find(".//m:LocalDebuggerWorkingDirectory", ns).text, "$(OutDir)")
        target = tree.find("m:Target[@Name='CopyGameResources']", ns)
        self.assertEqual(target.attrib["AfterTargets"], "Build")
        copy = target.find("m:Copy", ns)
        self.assertEqual(copy.attrib["SkipUnchangedFiles"], "true")
        self.assertIn("%(RecursiveDir)", copy.attrib["DestinationFiles"])

    def test_cpp_sources_keep_cp949_crlf_and_no_bom(self):
        for path in (ROOT / "src/RebornBoss").iterdir():
            if path.suffix not in {".cpp", ".h"}:
                continue
            data = path.read_bytes()
            self.assertFalse(data.startswith(b"\xef\xbb\xbf"), str(path))
            data.decode("cp949")
            self.assertEqual(data.count(b"\n"), data.count(b"\r\n"), str(path))

    def test_python_tools_are_syntactically_valid(self):
        for path in (ROOT / "tools").glob("*.py"):
            ast.parse(path.read_text(), filename=str(path))


if __name__ == "__main__":
    unittest.main()
