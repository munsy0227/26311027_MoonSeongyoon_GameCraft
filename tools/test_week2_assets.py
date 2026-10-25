import ast
from array import array
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import wave
import xml.etree.ElementTree as ET

from PIL import Image
from build_character_assets import check_border
from build_week2_assets import centered, inspect_wav, make_sfx, pcm_bytes, RATE, ROOT


class Week2AssetsTest(unittest.TestCase):
    def test_image_jobs_have_complete_sizes_and_sources(self):
        jobs = json.loads((ROOT / "art_sources/week2/asset_jobs.json").read_text())["jobs"]
        self.assertEqual(len(jobs), 22)
        self.assertEqual(len({j["path"] for j in jobs}), 22)
        for job in jobs:
            self.assertTrue((ROOT / "art_sources/week2/generated" / (job["name"] + ".png")).is_file())
            if job["columns"] > 1:
                self.assertEqual(job["size"], [job["cell"][0] * job["columns"], job["cell"][1]])

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
