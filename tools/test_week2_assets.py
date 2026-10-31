import ast
from array import array
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
from build_week2_assets import centered, export_image, inspect_wav, make_sfx, pcm_bytes, runtime_paths, ui_source, RATE, ROOT


class Week2AssetsTest(unittest.TestCase):
    def test_image_jobs_have_complete_sizes_and_sources(self):
        jobs = json.loads((ROOT / "art_sources/week2/asset_jobs.json").read_text())["jobs"]
        self.assertEqual(len(jobs), 24)
        self.assertEqual(len({j["path"] for j in jobs}), 24)
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

    def test_pcm_peak_is_normalized_without_clipping(self):
        pcm = array("h")
        pcm.frombytes(pcm_bytes([array("f", [0, 5, -5, 0])], 0.6))
        self.assertAlmostEqual(max(abs(x) for x in pcm) / 32767, 0.6, places=4)
        self.assertEqual(pcm[0], 0)
        self.assertEqual(pcm[-1], 0)

    def test_generated_effect_is_valid_pcm_with_silent_edges(self):
        with tempfile.TemporaryDirectory() as folder:
            with patch("build_week2_assets.RES", Path(folder)):
                make_sfx(("SFX_MENU_MOVE", "menu_move", 0.08, 660, 660, 0))
            report = inspect_wav(Path(folder) / "audio/sfx/menu_move.wav", False)
        self.assertEqual(report["seconds"], 0.08)
        self.assertEqual(report["boundary_jump"], 0)

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
    def test_runtime_resources_exist_and_are_tracked_in_git_checkouts(self):
        paths = {"src/RebornBoss/resources/" + path for path in runtime_paths()}
        self.assertTrue(paths)
        for path in paths:
            self.assertTrue((ROOT / path).is_file(), path)
        if (ROOT / ".git").exists() and shutil.which("git"):
            tracked = set(subprocess.check_output(
                ["git", "ls-files", "-z", "--", "src/RebornBoss/resources"], cwd=ROOT
            ).decode().split("\0"))
            self.assertFalse(paths - tracked, sorted(paths - tracked))

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
