#!/usr/bin/env python3
"""Export approved image sources and MIDI-instrument WAVs; --check never rewrites them."""
import argparse
from array import array
import hashlib
import json
import math
from pathlib import Path
import re
import sys
import wave

from PIL import Image, ImageDraw, ImageOps
from build_character_assets import align_cells, check_border, cut_grid, pixel_palette, save_png, sheet, write_json

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "art_sources/week2"
RES = ROOT / "src/RebornBoss/resources"
from build_midi_audio import BGM, SFX, RATE, build_audio, build_all


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def centered(cells, size, margin=2, pixel=False):
    scale = min((size[0] - 2 * margin) / max(c.width for c in cells),
                (size[1] - 2 * margin) / max(c.height for c in cells))
    frames = []
    for cell in cells:
        cell = cell.resize((max(1, round(cell.width * scale)), max(1, round(cell.height * scale))),
                           Image.Resampling.NEAREST if pixel else Image.Resampling.LANCZOS)
        canvas = Image.new("RGBA", size)
        canvas.alpha_composite(cell, ((size[0] - cell.width) // 2, (size[1] - cell.height) // 2))
        frames.append(canvas)
    return frames


def export_image(job):
    image = Image.open(ART / "generated" / (job["name"] + ".png")).convert("RGBA")
    if job.get("procedural"):
        assert image.size == tuple(job["size"]), job["id"]
    elif job["columns"] > 1:
        pixel = job["id"].startswith("ENM_")
        cells = cut_grid(image, job["columns"], 1, pixel=pixel)
        if pixel:
            frames = pixel_palette(align_cells(cells, tuple(job["cell"]), pixel=True))
        else:
            frames = centered(cells, tuple(job["cell"]))
        image = sheet(frames, job["columns"])
    elif job["transparent"]:
        image.putalpha(image.getchannel("A").point(lambda a: 0 if a < 64 else min(255, round(a * 255 / 254))))
        box = image.getchannel("A").getbbox()
        if box is None:
            raise ValueError("Empty generated image: " + job["name"])
        image = image.crop(box)
        if job["id"] == "UI_DIALOGUE":
            image = image.resize(tuple(job["size"]), Image.Resampling.LANCZOS)
        else:
            image = centered([image], tuple(job["size"]), margin=1, pixel=True)[0]
            image.putalpha(image.getchannel("A").point(lambda a: 255 if a >= 128 else 0))
    else:
        image = ImageOps.fit(image, tuple(job["size"]), method=Image.Resampling.LANCZOS)
        image.putalpha(255)
    save_png(image, RES / job["path"])


def ui_source(job):
    size = tuple(job["size"])
    kind = job["procedural"]
    if kind == "panel":
        return Image.new("RGBA", size, "#1a2030")
    if kind == "white":
        return Image.new("RGBA", size, "white")
    if kind == "lock":
        image = Image.new("RGBA", (size[0] * 4, size[1] * 4))
        draw = ImageDraw.Draw(image)
        draw.arc((24, 8, 72, 72), 180, 360, fill="#bfc9d8", width=12)
        draw.rounded_rectangle((12, 40, 84, 92), radius=12, fill="#d6dce5", outline="#9ba9be", width=4)
        draw.ellipse((40, 52, 56, 68), fill="#1a2030")
        draw.rectangle((44, 64, 52, 80), fill="#1a2030")
        return image.resize(size, Image.Resampling.LANCZOS)
    assert kind in {"surface", "dialogue"}, kind
    image = Image.new("RGBA", (size[0] * 4, size[1] * 4))
    fill = "white" if kind == "surface" else (26, 32, 48, 248)
    ImageDraw.Draw(image).rounded_rectangle((0, 0, image.width - 1, image.height - 1), radius=48, fill=fill)
    return image.resize(size, Image.Resampling.LANCZOS)


def make_ui_sources(jobs):
    for job in jobs:
        if job.get("procedural"):
            save_png(ui_source(job), ART / "generated" / (job["name"] + ".png"))


def make_bgm(spec):
    asset_id, name, seconds = spec[:3]
    return build_audio(asset_id, name, seconds, RES / "audio/bgm" / (name + ".wav"))


def make_sfx(spec):
    asset_id, name, seconds = spec[:3]
    return build_audio(asset_id, name, seconds, RES / "audio/sfx" / (name + ".wav"))


def inspect_wav(path, bgm):
    with wave.open(str(path), "rb") as stream:
        channels, width, rate, frames, compression, _ = stream.getparams()
        assert channels == (2 if bgm else 1) and width == 2 and rate == RATE and compression == "NONE", path
        data = array("h", stream.readframes(frames))
    if sys.byteorder != "little":
        data.byteswap()
    peak = max(abs(value) for value in data) / 32768
    rms = math.sqrt(sum((value / 32768) ** 2 for value in data) / len(data))
    assert 0.05 < peak < 0.95 and rms > 0.005, path
    jump = max(abs(data[c] - data[-channels + c]) / 32768 for c in range(channels))
    assert jump < (0.06 if bgm else 0.001), (path, jump)
    seconds = frames / rate
    assert (30 <= seconds <= 90) if bgm else (0.05 <= seconds <= 3.0), path
    return {"seconds": seconds, "channels": channels, "peak": round(peak, 5),
            "rms": round(rms, 5), "boundary_jump": round(jump, 5)}


def reused_images():
    result = []
    for character, cell in [("leon", [32, 48]), ("lillian", [64, 64]), ("vivian", [64, 64]), ("eleanor", [64, 64])]:
        result.append({"id": "CHR_" + character.upper(), "path": f"images/characters/{character}_sheet.png",
                       "size": [cell[0] * 4, cell[1] * (3 if character == "leon" else 2)],
                       "origin": "character_assets_v1", "runtime_variant": f"images/characters/{character}_animations.png"})
        result.append({"id": "POR_" + character.upper(), "path": f"images/portraits/{character}_portrait.png",
                       "size": [320, 480], "origin": "character_assets_v1"})
    return result


def runtime_paths():
    source = (ROOT / "src/RebornBoss/GameResources.cpp").read_bytes().decode("cp949")
    return set(re.findall(r'"resources/([^"\n]+)"', source))


def make_catalog(jobs):
    catalog = reused_images()
    for job in jobs:
        catalog.append({key: job[key] for key in ["id", "path", "size", "columns", "cell", "transparent"]})
        origin = "Original procedural UI in tools/build_week2_assets.py" if job.get("procedural") else "OpenAI built-in image_gen"
        catalog[-1].update(origin=origin, source=f"art_sources/week2/generated/{job['name']}.png")
    for asset_id, name, seconds, *_ in BGM + SFX:
        bgm = asset_id.startswith("BGM_")
        catalog.append({"id": asset_id, "path": f"audio/{'bgm' if bgm else 'sfx'}/{name}.wav",
                        "seconds": seconds, "origin": "Original authored MIDI score rendered with FluidSynth / MS Basic",
                        "source": f"art_sources/audio/midi/{name}.mid",
                        "audio_manifest": "art_sources/audio/manifest.json"})
    active = runtime_paths()
    for entry in catalog:
        entry["applied"] = entry["path"] in active or entry.get("runtime_variant") in active
        entry["status"] = "적용" if entry["applied"] else "편집 완료"
        entry["sha256"] = digest(RES / entry["path"])
        if "source" in entry:
            entry["source_sha256"] = digest(ROOT / entry["source"])
    return {"date": "2026-11-01", "windows_verified": False, "catalog": catalog}


def validate(manifest):
    active = runtime_paths()
    for relative in active:
        assert (RES / relative).is_file(), relative
    catalog = manifest["catalog"]
    jobs = json.loads((ART / "asset_jobs.json").read_text())["jobs"]
    expected = len(reused_images()) + len(jobs) + len(BGM) + len(SFX)
    assert len(catalog) == expected and len({e["id"] for e in catalog}) == expected
    audio = {}
    png_count = 0
    for entry in catalog:
        applied = entry["path"] in active or entry.get("runtime_variant") in active
        assert entry["applied"] == applied, entry["id"]
        assert entry["status"] == ("적용" if applied else "편집 완료"), entry["id"]
        path = RES / entry["path"]
        assert path.is_file() and digest(path) == entry["sha256"], path
        if "source" in entry:
            assert digest(ROOT / entry["source"]) == entry["source_sha256"], entry["source"]
        if path.suffix == ".wav":
            audio[entry["id"]] = inspect_wav(path, entry["id"].startswith("BGM_"))
            assert audio[entry["id"]]["seconds"] == entry["seconds"]
            continue
        png_count += 1
        image = Image.open(path)
        assert image.mode == "RGBA" and image.size == tuple(entry["size"]), path
        if entry.get("transparent"):
            assert image.getchannel("A").getextrema()[0] == 0, path
        if entry.get("columns", 1) > 1:
            width, height = entry["cell"]
            hashes = set()
            for index in range(entry["columns"]):
                frame = image.crop((index * width, 0, (index + 1) * width, height))
                check_border(frame)
                assert frame.getchannel("A").getbbox() is not None, path
                hashes.add(hashlib.sha256(frame.tobytes()).hexdigest())
            assert len(hashes) > 1, path
    assert png_count == len(reused_images()) + len(jobs) and len(audio) == len(BGM) + len(SFX)
    return {"images": png_count, "sounds": len(audio), "new_images": len(jobs),
            "applied": sum(e["applied"] for e in catalog),
            "runtime_images": sum(path.endswith(".png") for path in active),
            "runtime_sounds": sum(path.endswith(".wav") for path in active),
            "runtime_fonts": sum(path.endswith(".ttf") for path in active),
            "audio": audio, "windows_verified": False}


def preview(jobs):
    canvas = Image.new("RGBA", (1120, math.ceil(len(jobs) / 4) * 220), "#141927")
    draw = ImageDraw.Draw(canvas)
    for index, job in enumerate(jobs):
        x, y = (index % 4) * 280, (index // 4) * 220
        for by in range(y + 30, y + 205, 10):
            for bx in range(x + 10, x + 270, 10):
                draw.rectangle((bx, by, bx + 9, by + 9), fill="#30374a" if (bx + by) % 20 else "#242b3e")
        image = Image.open(RES / job["path"]).convert("RGBA")
        scale = min(260 / image.width, 170 / image.height)
        size = (max(1, round(image.width * scale)), max(1, round(image.height * scale)))
        image = image.resize(size, Image.Resampling.NEAREST if job["transparent"] else Image.Resampling.LANCZOS)
        canvas.alpha_composite(image, (x + (280 - size[0]) // 2, y + 32 + (170 - size[1]) // 2))
        draw.text((x + 10, y + 8), f"{job['id']}  {job['size'][0]}x{job['size'][1]}", fill="#f5e7bd")
    save_png(canvas, ART / "previews/asset_contact_sheet.png")


def build():
    jobs = json.loads((ART / "asset_jobs.json").read_text())["jobs"]
    make_ui_sources(jobs)
    for job in jobs:
        export_image(job)
    build_all(RES)
    manifest = make_catalog(jobs)
    report = validate(manifest)
    write_json(RES / "week2_assets.json", manifest)
    write_json(ART / "validation.json", report)
    preview(jobs)
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check current files without exporting")
    args = parser.parse_args()
    result = validate(json.loads((RES / "week2_assets.json").read_text())) if args.check else build()
    print(json.dumps({key: value for key, value in result.items() if key != "audio"}))
