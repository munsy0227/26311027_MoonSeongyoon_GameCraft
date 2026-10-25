#!/usr/bin/env python3
"""Export the generated character atlases with Pillow, then validate the game files."""
import argparse
import hashlib
import json
import re
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "art_sources/characters"
RES = ROOT / "src/RebornBoss/resources"
STATES = ["idle", "move_left", "move_right", "cast", "hit", "victory", "defeat", "appear", "exit"]
DURATIONS = [120, 100, 100, 90, 80, 140, 160, 120, 120]
LOOPS = {"idle", "move_left", "move_right", "victory"}


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def save_png(image, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path, format="PNG", optimize=True)


def transparent_boundaries(mask, count, axis):
    length = mask.size[axis]
    boundaries = [0]
    radius = max(1, round(length / count * 0.28))
    for index in range(1, count):
        expected = round(index * length / count)
        scores = []
        for position in range(max(boundaries[-1] + 1, expected - radius), min(length, expected + radius + 1)):
            box = (position, 0, position + 1, mask.height) if axis == 0 else (0, position, mask.width, position + 1)
            score = sum(mask.crop(box).histogram()[1:])
            scores.append((score, abs(position - expected), position))
        boundaries.append(min(scores)[2])
    return boundaries + [length]


def remove_specks(image):
    alpha = image.getchannel("A")
    size = (max(1, image.width // 4), max(1, image.height // 4))
    mask = alpha.resize(size, Image.Resampling.NEAREST).point(lambda value: 255 if value >= 64 else 0)
    data = bytearray(mask.tobytes())
    width, height = size
    parts = []
    for seed in range(len(data)):
        if not data[seed]:
            continue
        queue, part = [seed], []
        data[seed] = 0
        while queue:
            index = queue.pop()
            part.append(index)
            x, y = index % width, index // width
            for ny in range(max(0, y - 1), min(height, y + 2)):
                for nx in range(max(0, x - 1), min(width, x + 2)):
                    neighbor = ny * width + nx
                    if data[neighbor]:
                        data[neighbor] = 0
                        queue.append(neighbor)
        parts.append(part)
    if not parts:
        return image
    minimum = max(8, max(map(len, parts)) // 100)
    keep = bytearray(width * height)
    for part in parts:
        if len(part) >= minimum:
            for index in part:
                keep[index] = 255
    mask = Image.frombytes("L", size, bytes(keep)).resize(image.size, Image.Resampling.NEAREST)
    image.putalpha(ImageChops.multiply(alpha, mask.filter(ImageFilter.MaxFilter(9))))
    return image


def cut_grid(image, columns, rows, pixel=False, portrait=False):
    image = image.convert("RGBA")
    mask = image.getchannel("A").point(lambda value: 255 if value >= 128 else 0)
    vertical = ([round(index * image.height / rows) for index in range(rows + 1)] if portrait
                else transparent_boundaries(mask, rows, 1))
    cells = []
    for row in range(rows):
        row_mask = mask.crop((0, vertical[row], image.width, vertical[row + 1]))
        horizontal = ([round(index * image.width / columns) for index in range(columns + 1)] if portrait
                      else transparent_boundaries(row_mask, columns, 0))
        for column in range(columns):
            box = (horizontal[column], vertical[row], horizontal[column + 1], vertical[row + 1])
            cell = image.crop(box)
            if portrait:
                # Busts meet at atlas edges; remove the neighboring shoulder overscan.
                inset = max(1, round(cell.width * 0.035))
                cell = cell.crop((inset, 0, cell.width - inset, cell.height))
            alpha = cell.getchannel("A")
            # The generator leaves near-zero matte pixels outside otherwise transparent figures.
            if pixel:
                alpha = alpha.point(lambda value: 255 if value >= 128 else 0)
            else:
                alpha = alpha.point(lambda value: 0 if value < 64 else min(255, round(value * 255 / 253)))
            cell.putalpha(alpha)
            if not pixel:
                cell = remove_specks(cell)
                alpha = cell.getchannel("A")
            ink = alpha.point(lambda value: 255 if value >= 64 else 0).getbbox()
            if ink is None:
                raise ValueError(f"Empty source cell: column {column + 1}, row {row + 1}")
            cells.append(cell.crop(ink))
    return cells


def align_cells(cells, size, pixel=False, top=False):
    margin = 2 if pixel else 12
    width, height = size
    scale = min((width - 2 * margin) / max(cell.width for cell in cells),
                (height - 2 * margin) / max(cell.height for cell in cells))
    result = []
    for cell in cells:
        resized = cell.resize((max(1, round(cell.width * scale)), max(1, round(cell.height * scale))),
                              Image.Resampling.NEAREST if pixel else Image.Resampling.LANCZOS)
        frame = Image.new("RGBA", size)
        frame.alpha_composite(resized, ((width - resized.width) // 2,
                                       margin if top else height - margin - resized.height))
        result.append(frame)
    return result


def sheet(frames, columns):
    width, height = frames[0].size
    result = Image.new("RGBA", (columns * width, ((len(frames) + columns - 1) // columns) * height))
    for index, frame in enumerate(frames):
        result.paste(frame, ((index % columns) * width, (index // columns) * height))
    return result


def pixel_palette(frames):
    atlas = sheet(frames, 4)
    alpha = atlas.getchannel("A")
    atlas = atlas.convert("RGB").quantize(colors=48, method=Image.Quantize.MEDIANCUT,
                                        dither=Image.Dither.NONE).convert("RGBA")
    atlas.putalpha(alpha)
    width, height = frames[0].size
    return [atlas.crop(((index % 4) * width, (index // 4) * height,
                        (index % 4 + 1) * width, (index // 4 + 1) * height))
            for index in range(len(frames))]


def faded(frame, opacity):
    image = frame.copy()
    image.putalpha(frame.getchannel("A").point(lambda value: round(value * opacity / 255)))
    return image


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_border(image, margin=2):
    alpha = image.getchannel("A")
    for box in [(0, 0, image.width, margin), (0, image.height - margin, image.width, image.height),
                (0, 0, margin, image.height), (image.width - margin, 0, image.width, image.height)]:
        if alpha.crop(box).getbbox() is not None:
            raise ValueError("A sprite touches its transparent safety margin")


def validate(manifest):
    for character in manifest["characters"]:
        width, height = character["cell"]
        for path in character["portraits"].values():
            image = Image.open(RES / path)
            assert image.mode == "RGBA" and image.size == (320, 480), path
        for path in character["poses"].values():
            image = Image.open(RES / path)
            assert image.mode == "RGBA" and image.size == (640, 960), path
        atlas = Image.open(RES / character["atlas"])
        assert atlas.mode == "RGBA" and atlas.size == (width * 4, height * len(STATES))
        basic = Image.open(RES / character["basic_sheet"])
        assert basic.mode == "RGBA" and basic.size == (width * 4, height * (3 if character["id"] == "leon" else 2))
        alias = Image.open(RES / f"images/portraits/{character['id']}_portrait.png")
        assert alias.tobytes() == Image.open(RES / character["portraits"]["neutral"]).tobytes()
        for index, state in enumerate(STATES):
            info = character["animations"][state]
            assert info["row"] == index and info["frame_count"] == 4 and info["frame_ms"] > 0
            strip = Image.open(RES / info["sheet"])
            assert strip.size == (width * 4, height)
            assert strip.tobytes() == atlas.crop((0, index * height, width * 4, (index + 1) * height)).tobytes()
            hashes = set()
            for number, path in enumerate(info["frames"]):
                frame = Image.open(RES / path)
                assert frame.mode == "RGBA" and frame.size == (width, height), path
                check_border(frame)
                if state not in {"appear", "exit"}:
                    assert set(frame.getchannel("A").tobytes()) <= {0, 255}, path
                    colors = frame.convert("RGB").getcolors(maxcolors=49)
                    assert colors is not None and len(colors) <= 48, path
                crop = atlas.crop((number * width, index * height, (number + 1) * width, (index + 1) * height))
                assert crop.tobytes() == frame.tobytes(), path
                assert frame.getchannel("A").getbbox() is not None or state in {"appear", "exit"}
                hashes.add(hashlib.sha256(frame.tobytes()).hexdigest())
            if state in {"cast", "hit", "defeat"}:
                assert len(hashes) > 1, f"{character['id']}: {state} needs actual animation changes"
        for path in [character["atlas"], character["basic_sheet"], *character["portraits"].values(),
                     *character["poses"].values()]:
            assert (RES / path).is_file(), path
    pngs = list((RES / "images").rglob("*.png"))
    return {"characters": len(manifest["characters"]), "png_files": len(pngs),
            "expressions": 24, "body_poses": 32, "combat_states": 36, "combat_frames": 144,
            "all_rgba": all(Image.open(path).mode == "RGBA" for path in pngs)}


def preview_gif(character, frames):
    previews = []
    durations = []
    for index, state in enumerate(STATES):
        for frame in frames[index * 4:(index + 1) * 4]:
            canvas = Image.new("RGBA", (320, 340), "#223448")
            draw = ImageDraw.Draw(canvas)
            for x in range(0, 320, 20):
                for y in range(0, 340, 20):
                    if (x // 20 + y // 20) % 2:
                        draw.rectangle((x, y, x + 19, y + 19), fill="#2c4055")
            scaled = frame.resize((frame.width * 4, frame.height * 4), Image.Resampling.NEAREST)
            canvas.alpha_composite(scaled, ((320 - scaled.width) // 2, 42 + (280 - scaled.height) // 2))
            ImageDraw.Draw(canvas).text((12, 12), character["id"] + " / " + state,
                                       fill="white", font_size=18)
            previews.append(canvas.convert("RGB"))
            durations.append(DURATIONS[index])
    path = ART / "previews" / (character["id"] + "_combat.gif")
    path.parent.mkdir(parents=True, exist_ok=True)
    previews[0].save(path, save_all=True, append_images=previews[1:], duration=durations, loop=0)


def preview_html(manifest):
    template = """<!doctype html><html lang="ko"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>캐릭터 리소스 완성본</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#f2f4f8;color:#1b263b;font:16px/1.5 system-ui,sans-serif}
main{max-width:1280px;margin:auto;padding:28px}h1{margin:0 0 8px}p{margin:8px 0 20px}
#cards{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:24px}
article{background:white;border:1px solid #dce1ea;border-radius:14px;padding:20px;min-width:0}
h2{margin:0 0 14px;font-size:21px}.views{display:flex;gap:12px;align-items:flex-start}
.portrait{width:42%;height:300px;object-fit:contain;background:repeating-conic-gradient(#edf0f3 0 25%,white 0 50%) 0/20px 20px}
canvas{width:54%;height:auto;border:1px solid #dce1ea;image-rendering:pixelated}
label{display:block;font-size:14px;margin:12px 0}select{max-width:100%;padding:7px;margin-left:6px;font:inherit}
.poses{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:5px}
.poses img{width:100%;height:140px;object-fit:contain;background:#f3f5f8}.poses a{font-size:12px;text-align:center;color:#304f8c;text-decoration:none}
.files{display:flex;gap:12px;flex-wrap:wrap;font-size:14px}a{color:#285bb0}
@media(max-width:850px){#cards{grid-template-columns:1fr}main{padding:14px}.portrait{height:260px}}
</style><main><h1>캐릭터 리소스 완성본</h1>
<p>캐릭터별 감정 6종 · 전신 동작 8종 · 전투 9상태 × 4프레임. 전투 화면은 실제 PNG를 재생하는 파일 확인용 미리보기입니다.</p>
<div id="cards"></div></main><script>
const data=__MANIFEST__;
const base='../../src/RebornBoss/resources/';
const emotionNames={neutral:'기본',happy:'미소',angry:'분노',surprised:'놀람',determined:'결의',hurt:'피로·피격'};
const stateNames={idle:'대기',move_left:'왼쪽 이동',move_right:'오른쪽 이동',cast:'공격·시전',hit:'피격',victory:'승리',defeat:'패배',appear:'등장',exit:'퇴장'};
for(const c of data.characters){
 const card=document.createElement('article');const title=document.createElement('h2');title.textContent=c.name;card.append(title);
 const views=document.createElement('div');views.className='views';
 const portrait=document.createElement('img');portrait.className='portrait';portrait.alt=c.name+' 감정 초상화';portrait.src=base+c.portraits.neutral;
 const canvas=document.createElement('canvas');canvas.width=320;canvas.height=340;canvas.setAttribute('aria-label',c.name+' 전투 애니메이션');
 views.append(portrait,canvas);card.append(views);
 const emotionLabel=document.createElement('label');emotionLabel.append('감정');const em=document.createElement('select');
 for(const [id,name] of Object.entries(emotionNames)){const opt=new Option(name,id);em.add(opt)}em.onchange=()=>portrait.src=base+c.portraits[em.value];emotionLabel.append(em);card.append(emotionLabel);
 const stateLabel=document.createElement('label');stateLabel.append('전투 동작');const states=document.createElement('select');
 for(const [id,name] of Object.entries(stateNames)){states.add(new Option(name,id))}stateLabel.append(states);card.append(stateLabel);
 const bgLabel=document.createElement('label');bgLabel.append('배경');const backgrounds=document.createElement('select');
 for(const [value,name] of [['#254735','숲'],['#23435d','호수'],['#292c4b','신전'],['#e9edf4','밝은 배경'],['#161c25','어두운 배경']]){backgrounds.add(new Option(name,value))}bgLabel.append(backgrounds);card.append(bgLabel);
 const poseTitle=document.createElement('p');poseTitle.textContent='전신 동작 (누르면 원본 PNG)';card.append(poseTitle);
 const poses=document.createElement('div');poses.className='poses';for(const [id,path] of Object.entries(c.poses)){const a=document.createElement('a');a.href=base+path;a.target='_blank';const img=document.createElement('img');img.src=base+path;img.alt=c.name+' '+stateNames[id];a.append(img,document.createTextNode(stateNames[id]));poses.append(a)}card.append(poses);
 const files=document.createElement('p');files.className='files';for(const [label,path] of [['기본 시트',c.basic_sheet],['전체 애니메이션 시트',c.atlas],['프레임 정보',c.metadata]]){const a=document.createElement('a');a.textContent=label;a.href=base+path;files.append(a)}card.append(files);
 document.getElementById('cards').append(card);
 const image=new Image();image.src=base+c.atlas;const ctx=canvas.getContext('2d');ctx.imageSmoothingEnabled=false;
 let start=performance.now();states.onchange=()=>{start=performance.now()};
 function draw(now){const info=c.animations[states.value];let index=Math.floor((now-start)/info.frame_ms);index=info.loop?index%4:Math.min(3,index);ctx.fillStyle=backgrounds.value;ctx.fillRect(0,0,320,340);ctx.fillStyle='#ffffff14';for(let y=0;y<340;y+=32){ctx.fillRect(0,y,320,1)}
  if(image.complete&&image.naturalWidth){const w=c.cell[0],h=c.cell[1];ctx.drawImage(image,index*w,info.row*h,w,h,(320-w*4)/2,42+(280-h*4)/2,w*4,h*4)}
  requestAnimationFrame(draw)}requestAnimationFrame(draw);
}
</script></html>"""
    (ART / "preview.html").write_text(template.replace("__MANIFEST__", json.dumps(manifest, ensure_ascii=False)),
                                     encoding="utf-8")


def build():
    spec = json.loads((ART / "production_v1.json").read_text(encoding="utf-8"))
    sources = {}
    for job in spec["jobs"]:
        source = (ROOT / job["source"]).resolve()
        if not source.is_relative_to(ART.resolve()) or not re.fullmatch("[a-z_]+", job["character"]):
            raise ValueError("Invalid source path or character id")
        with Image.open(source) as image:
            sources[(job["character"], job["kind"])] = cut_grid(
                image, *job["grid"], pixel=job["kind"] == "combat", portrait=job["kind"] == "expressions")
    manifest = {"version": 1, "date": spec["date"], "characters": []}
    for character in spec["cast"]:
        cid = character["id"]
        portraits = align_cells(sources[(cid, "expressions")], (320, 480), top=True)
        poses = align_cells(sources[(cid, "poses")], (640, 960))
        combat = pixel_palette(align_cells(sources[(cid, "combat")], tuple(character["cell"]), pixel=True))
        entry = {"id": cid, "name": character["name"], "cell": character["cell"],
                 "pivot": [value // 2 for value in character["cell"]],
                 "atlas": f"images/characters/{cid}_animations.png",
                 "basic_sheet": f"images/characters/{cid}_sheet.png",
                 "metadata": f"images/characters/{cid}_animations.json",
                 "portraits": {}, "poses": {}, "animations": {}}
        for name, frame in zip(spec["expressions"], portraits, strict=True):
            path = f"images/portraits/{cid}_portrait_{name}.png"
            save_png(frame, RES / path)
            entry["portraits"][name] = path
        save_png(portraits[0], RES / f"images/portraits/{cid}_portrait.png")
        for name, frame in zip(spec["poses"], poses, strict=True):
            path = f"images/poses/{cid}/{cid}_{name}.png"
            save_png(frame, RES / path)
            entry["poses"][name] = path
        combat += [faded(combat[index], opacity) for index, opacity in enumerate([0, 96, 192, 255])]
        combat += [faded(combat[index], opacity) for index, opacity in enumerate([255, 192, 96, 0])]
        save_png(sheet(combat, 4), RES / entry["atlas"])
        basic = combat[:12] if cid == "leon" else combat[:4] + combat[12:16]
        save_png(sheet(basic, 4), RES / entry["basic_sheet"])
        for index, state in enumerate(STATES):
            frames = combat[index * 4:(index + 1) * 4]
            info = {"row": index, "frame_count": 4, "frame_ms": DURATIONS[index], "loop": state in LOOPS,
                    "hold_last": state not in LOOPS,
                    "sheet": f"images/characters/{cid}/{state}_sheet.png", "frames": []}
            save_png(sheet(frames, 4), RES / info["sheet"])
            for number, frame in enumerate(frames):
                path = f"images/characters/{cid}/{state}_{number:02}.png"
                save_png(frame, RES / path)
                info["frames"].append(path)
            entry["animations"][state] = info
        write_json(RES / entry["metadata"], entry)
        preview_gif(entry, combat)
        save_png(sheet(portraits, 3), ART / "previews" / (cid + "_expressions.png"))
        save_png(sheet([frame.resize((320, 480), Image.Resampling.LANCZOS) for frame in poses], 4),
                 ART / "previews" / (cid + "_poses.png"))
        manifest["characters"].append(entry)
    write_json(RES / "character_assets.json", manifest)
    preview_html(manifest)
    report = validate(manifest)
    report["source_sha256"] = {job["source"]: digest(ROOT / job["source"]) for job in spec["jobs"]}
    report["export_sha256"] = {str(path.relative_to(RES)): digest(path)
                               for path in sorted((RES / "images").rglob("*.png"))}
    write_json(ART / "validation_v1.json", report)
    print(json.dumps({key: value for key, value in report.items() if not key.endswith("sha256")}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Validate existing exports without rewriting images")
    args = parser.parse_args()
    if args.check:
        print(json.dumps(validate(json.loads((RES / "character_assets.json").read_text(encoding="utf-8")))))
    else:
        build()
