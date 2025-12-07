#!/usr/bin/env python3

from pathlib import Path
import json
import math
from typing import Dict, List, Tuple

from PIL import Image, ImageDraw, ImageFilter


BASE_WIDTH = 1280
BASE_HEIGHT = 720
SLICE_CORNER_MIN = 16
SLICE_CORNER_MAX = 24
SLICE_EDGE_MIN = 8
SLICE_EDGE_MAX = 12


def ensure_out_dir() -> Path:
    root = Path(__file__).resolve().parents[1]
    out_dir = root / "resources" / "images" / "generated"
    out_dir.mkdir(parents=True, exist_ok=True)
    return out_dir


def gradient_rect(size: Tuple[int, int], top: Tuple[int, int, int, int],
                  bottom: Tuple[int, int, int, int]) -> Image.Image:
    width, height = size
    base = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(base)
    for y in range(height):
        t = y / max(height - 1, 1)
        r = int(top[0] + (bottom[0] - top[0]) * t)
        g = int(top[1] + (bottom[1] - top[1]) * t)
        b = int(top[2] + (bottom[2] - top[2]) * t)
        a = int(top[3] + (bottom[3] - top[3]) * t)
        draw.line([(0, y), (width, y)], fill=(r, g, b, a))
    return base


def make_crt_overlay() -> Tuple[str, Image.Image]:
    width, height = BASE_WIDTH, BASE_HEIGHT
    overlay = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    scanline_color = (0, 0, 0, 40)
    for y in range(0, height, 3):
        draw.rectangle([(0, y), (width, y + 1)], fill=scanline_color)
    vignette = Image.new("L", (width, height), 0)
    vig_draw = ImageDraw.Draw(vignette)
    radius = int(math.hypot(width, height))
    vig_draw.ellipse(
        [(-radius // 2, -radius // 2), (width + radius // 2,
                                        height + radius // 2)],
        fill=180)
    vignette = vignette.filter(ImageFilter.GaussianBlur(radius=width / 8))
    overlay.putalpha(vignette)
    return "crt_overlay", overlay


def make_bezel() -> Tuple[str, Image.Image]:
    width, height = BASE_WIDTH, BASE_HEIGHT
    outer_radius = 32
    inner_margin = 52
    glass_margin = inner_margin + 10

    bezel = Image.new("RGBA", (width, height), (0, 0, 0, 0))

    # Classic 90s beige plastic shell
    shell = gradient_rect((width, height), (226, 220, 205, 255),
                          (202, 195, 178, 255))
    shell_mask = Image.new("L", (width, height), 0)
    shell_draw = ImageDraw.Draw(shell_mask)
    shell_draw.rounded_rectangle([0, 0, width, height],
                                 radius=outer_radius,
                                 fill=255)
    shell_draw.rounded_rectangle(
        [inner_margin, inner_margin, width - inner_margin,
         height - inner_margin],
        radius=outer_radius - 10,
        fill=0)
    shell.putalpha(shell_mask)
    bezel.alpha_composite(shell)

    ring_specs = [
        (12, (235, 230, 218, 255), (214, 207, 190, 255)),
        (24, (245, 240, 228, 255), (224, 217, 202, 255)),
        (34, (255, 252, 242, 255), (232, 225, 210, 255)),
    ]
    for inset, top, bottom in ring_specs:
        band = gradient_rect(
            (width - inset * 2, height - inset * 2), top, bottom)
        band_mask = Image.new("L", (width, height), 0)
        band_draw = ImageDraw.Draw(band_mask)
        band_draw.rounded_rectangle(
            [inset, inset, width - inset, height - inset],
            radius=max(6, outer_radius - inset // 3),
            fill=255)
        band_draw.rounded_rectangle(
            [inset + 10, inset + 10, width - inset - 10,
             height - inset - 10],
            radius=max(4, outer_radius - inset // 2 - 4),
            fill=0)
        band.putalpha(band_mask.crop(
            (inset, inset, width - inset, height - inset)))
        bezel.alpha_composite(band, (inset, inset))

    lip_inset = inner_margin - 6
    lip = gradient_rect((width - lip_inset * 2, height - lip_inset * 2),
                        (216, 209, 193, 255), (190, 183, 168, 255))
    lip_mask = Image.new("L", (width, height), 0)
    lip_draw = ImageDraw.Draw(lip_mask)
    lip_draw.rounded_rectangle(
        [lip_inset, lip_inset, width - lip_inset, height - lip_inset],
        radius=outer_radius - 12,
        fill=255)
    lip_draw.rounded_rectangle(
        [glass_margin, glass_margin, width - glass_margin,
         height - glass_margin],
        radius=outer_radius - 18,
        fill=0)
    lip.putalpha(lip_mask.crop(
        (lip_inset, lip_inset, width - lip_inset, height - lip_inset)))
    bezel.alpha_composite(lip, (lip_inset, lip_inset))

    shadow = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    shadow_mask = Image.new("L", (width, height), 0)
    sh_draw = ImageDraw.Draw(shadow_mask)
    sh_draw.rounded_rectangle(
        [glass_margin, glass_margin, width - glass_margin,
         height - glass_margin],
        radius=outer_radius - 20,
        fill=220)
    shadow_mask = shadow_mask.filter(ImageFilter.GaussianBlur(radius=12))
    shadow.putalpha(shadow_mask)
    bezel.alpha_composite(shadow)

    glass = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glass_mask = Image.new("L", (width, height), 0)
    g_draw = ImageDraw.Draw(glass_mask)
    g_draw.rounded_rectangle(
        [glass_margin, glass_margin, width - glass_margin,
         height - glass_margin],
        radius=outer_radius - 22,
        fill=180)
    glass_mask = glass_mask.filter(ImageFilter.GaussianBlur(radius=6))
    glare = Image.new("RGBA", (width, height), (255, 255, 255, 0))
    glare_draw = ImageDraw.Draw(glare)
    glare_draw.polygon(
        [
            (glass_margin + 40, glass_margin + 10),
            (glass_margin + width * 0.35, glass_margin + 30),
            (glass_margin + 20, glass_margin + height * 0.35),
        ],
        fill=(255, 255, 255, 40))
    glare = glare.filter(ImageFilter.GaussianBlur(radius=14))
    glass.alpha_composite(glare)
    glass.putalpha(glass_mask)
    bezel.alpha_composite(glass)

    screw_color = (120, 113, 100, 200)
    screw_shadow = (0, 0, 0, 90)
    screw_positions = [
        (outer_radius + 12, outer_radius + 12),
        (width - outer_radius - 12, outer_radius + 12),
        (outer_radius + 12, height - outer_radius - 12),
        (width - outer_radius - 12, height - outer_radius - 12),
    ]
    for x, y in screw_positions:
        screw = Image.new("RGBA", (12, 12), (0, 0, 0, 0))
        sc_draw = ImageDraw.Draw(screw)
        sc_draw.ellipse([1, 1, 11, 11], fill=screw_shadow)
        sc_draw.ellipse([0, 0, 10, 10], fill=screw_color)
        sc_draw.line([(2, 6), (9, 6)], fill=(40, 44, 48, 255), width=2)
        sc_draw.line([(5, 3), (5, 9)], fill=(40, 44, 48, 255), width=2)
        bezel.alpha_composite(screw, (int(x) - 6, int(y) - 6))

    final_alpha = Image.new("L", (width, height), 0)
    final_draw = ImageDraw.Draw(final_alpha)
    final_draw.rounded_rectangle([0, 0, width, height],
                                 radius=outer_radius,
                                 fill=255)
    final_draw.rounded_rectangle(
        [glass_margin - 6, glass_margin - 6, width - glass_margin + 6,
         height - glass_margin + 6],
        radius=outer_radius - 16,
        fill=0)
    bezel.putalpha(final_alpha)

    return "bezel", bezel


def make_window_chrome() -> Tuple[str, Image.Image]:
    size = 128
    # XP-style chrome: blue title, light body, subtle border
    chrome = gradient_rect((size, size), (232, 235, 239, 255),
                           (210, 214, 220, 255))
    draw = ImageDraw.Draw(chrome)
    draw.rectangle([(1, 1), (size - 2, size - 2)],
                   outline=(255, 255, 255, 190),
                   width=2)
    draw.rectangle([(0, 0), (size - 1, size - 1)],
                   outline=(90, 120, 170, 255),
                   width=1)
    title_h = 28
    title_bar = gradient_rect((size, title_h), (69, 137, 255, 255),
                              (39, 96, 205, 255))
    chrome.alpha_composite(title_bar, (0, 0))
    button_size = 14
    gap = 6
    bx = size - (button_size + gap) * 3
    by = (title_h - button_size) // 2
    colors = [(232, 73, 60, 255), (251, 201, 80, 255), (120, 200, 120, 255)]
    for i, color in enumerate(colors):
        rect = [
            bx + i * (button_size + gap), by, bx + i * (button_size + gap) +
            button_size, by + button_size
        ]
        draw.rounded_rectangle(rect, radius=3, fill=color)
    return "window_chrome", chrome


def make_desk_texture() -> Tuple[str, Image.Image]:
    size = 512
    base = Image.new("RGBA", (size, size), (144, 116, 90, 255))
    draw = ImageDraw.Draw(base)
    for y in range(0, size, 6):
        alpha = 18 + (y % 12)
        draw.line([(0, y), (size, y)], fill=(90, 70, 55, alpha), width=2)
    noise = Image.effect_noise((size, size), 8)
    noise = noise.convert("L").point(lambda p: int(p * 0.25))
    noise = noise.filter(ImageFilter.GaussianBlur(radius=0.6))
    base.putalpha(255)
    base = Image.composite(base, base, noise)
    return "desk_texture", base


def make_conveyor_stripes() -> Tuple[str, Image.Image]:
    width = 512
    height = 192
    base = Image.new("RGBA", (width, height), (28, 34, 42, 255))
    stripe = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(stripe)
    stripe_w = 32
    gap = 24
    offset = -height
    color = (80, 180, 220, 160)
    while offset < width + height:
        points = [(offset, 0), (offset + stripe_w, 0),
                  (offset - height + stripe_w, height),
                  (offset - height, height)]
        draw.polygon(points, fill=color)
        offset += stripe_w + gap
    stripe = stripe.filter(ImageFilter.GaussianBlur(radius=0.8))
    base.alpha_composite(stripe)
    belt = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    belt_draw = ImageDraw.Draw(belt)
    belt_draw.rectangle([(0, height // 2 - 8), (width, height // 2 + 8)],
                        fill=(18, 22, 30, 200))
    belt = belt.filter(ImageFilter.GaussianBlur(radius=1.4))
    base.alpha_composite(belt)
    return "conveyor_stripes", base


def make_timeline_icon(name: str, painter) -> Tuple[str, Image.Image]:
    size = 48
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    painter(ImageDraw.Draw(img), size)
    return name, img


def paint_conveyor_icon(draw: ImageDraw.ImageDraw, size: int) -> None:
    bg_top = (230, 235, 245, 255)
    bg_bottom = (205, 214, 230, 255)
    base = gradient_rect((size, size), bg_top, bg_bottom)
    draw.bitmap((0, 0), base, fill=None)
    belt_h = 16
    belt_y = size // 2 - belt_h // 2 + 6
    draw.rounded_rectangle([(6, belt_y), (size - 6, belt_y + belt_h)],
                           radius=6,
                           fill=(50, 60, 75, 255),
                           outline=(20, 28, 40, 255),
                           width=1)
    for x in range(8, size - 8, 10):
        draw.rounded_rectangle([(x, belt_y + 4), (x + 6, belt_y + belt_h - 4)],
                               radius=2,
                               fill=(90, 180, 220, 200))
    arrow_y = size // 2 - 10
    arrow_color = (30, 120, 200, 255)
    for i in range(3):
        x0 = 10 + i * 12
        pts = [(x0, arrow_y + 6), (x0 + 10, arrow_y), (x0 + 10, arrow_y + 12)]
        draw.polygon(pts, fill=arrow_color)
    draw.rounded_rectangle([(4, 4), (size - 5, size - 5)],
                           radius=8,
                           outline=(120, 140, 180, 255),
                           width=1)


def paint_box_icon(draw: ImageDraw.ImageDraw, size: int) -> None:
    bg = gradient_rect((size, size), (246, 236, 210, 255), (228, 212, 180, 255))
    draw.bitmap((0, 0), bg, fill=None)
    body = [(10, 16), (size - 10, size - 10)]
    draw.rounded_rectangle(body,
                           radius=6,
                           fill=(214, 160, 90, 255),
                           outline=(140, 90, 40, 255),
                           width=2)
    lid = [(10, 14), (size - 10, 28)]
    draw.rounded_rectangle(lid,
                           radius=4,
                           fill=(225, 182, 120, 255),
                           outline=(140, 90, 40, 255),
                           width=2)
    tape_x = size // 2 - 6
    draw.rectangle([(tape_x, 14), (tape_x + 12, size - 10)],
                   fill=(245, 225, 150, 255))
    draw.line([(tape_x + 6, 14), (tape_x + 6, size - 10)],
              fill=(200, 170, 110, 255),
              width=2)
    draw.rounded_rectangle([(4, 4), (size - 5, size - 5)],
                           radius=8,
                           outline=(150, 140, 120, 230),
                           width=1)


def paint_ready_icon(draw: ImageDraw.ImageDraw, size: int) -> None:
    bg = gradient_rect((size, size), (220, 240, 220, 255), (190, 225, 190, 255))
    draw.bitmap((0, 0), bg, fill=None)
    ring = [(6, 6), (size - 6, size - 6)]
    draw.ellipse(ring, fill=(120, 190, 120, 255), outline=(60, 120, 70, 255), width=2)
    check_pts = [(14, size // 2), (22, size // 2 + 10), (34, size // 2 - 10)]
    draw.line(check_pts[:2], fill=(255, 255, 255, 255), width=4)
    draw.line(check_pts[1:], fill=(255, 255, 255, 255), width=4)
    draw.rounded_rectangle([(4, 4), (size - 5, size - 5)],
                           radius=8,
                           outline=(90, 140, 100, 220),
                           width=1)


def paint_ship_icon(draw: ImageDraw.ImageDraw, size: int) -> None:
    bg = gradient_rect((size, size), (220, 230, 245, 255), (195, 210, 235, 255))
    draw.bitmap((0, 0), bg, fill=None)
    body_y = size // 2 + 4
    draw.rectangle([(10, body_y - 12), (size - 10, body_y + 6)],
                   fill=(70, 130, 200, 255),
                   outline=(40, 80, 130, 255),
                   width=2)
    draw.rectangle([(12, body_y - 20), (26, body_y - 12)],
                   fill=(220, 235, 255, 255),
                   outline=(60, 100, 150, 255),
                   width=2)
    wheel_y = body_y + 8
    for x in (18, size - 18):
        draw.ellipse([(x - 6, wheel_y - 6), (x + 6, wheel_y + 6)],
                     fill=(40, 40, 40, 255))
        draw.ellipse([(x - 3, wheel_y - 3), (x + 3, wheel_y + 3)],
                     fill=(180, 180, 180, 255))
    draw.rounded_rectangle([(4, 4), (size - 5, size - 5)],
                           radius=8,
                           outline=(120, 140, 170, 220),
                           width=1)


def pad_to_grid(size: Tuple[int, int], grid: int) -> Tuple[int, int]:
    w, h = size
    pad_w = ((w + grid - 1) // grid) * grid
    pad_h = ((h + grid - 1) // grid) * grid
    return pad_w, pad_h


def pack_atlas(inputs: Dict[str, Image.Image],
               grid: int = 32,
               max_width: int = 2048) -> Tuple[Image.Image, Dict[str, Dict]]:
    atlas_w = max_width
    x = 0
    y = 0
    row_h = 0
    placements: Dict[str, Dict] = {}

    loaded: List[Tuple[str, Image.Image]] = list(inputs.items())
    loaded.sort(key=lambda item: item[1].height, reverse=True)

    for name, img in loaded:
        pad_w, pad_h = pad_to_grid(img.size, grid)
        if x + pad_w > atlas_w:
            x = 0
            y += row_h
            row_h = 0
        placements[name] = {
            "x": x,
            "y": y,
            "w": img.width,
            "h": img.height,
            "pad_w": pad_w,
            "pad_h": pad_h,
        }
        x += pad_w
        row_h = max(row_h, pad_h)

    atlas_h = ((y + row_h + grid - 1) // grid) * grid
    atlas = Image.new("RGBA", (atlas_w, atlas_h), (0, 0, 0, 0))
    for name, img in loaded:
        p = placements[name]
        atlas.alpha_composite(img, (p["x"], p["y"]))

    return atlas, placements


def write_atlas(out_dir: Path,
               inputs: Dict[str, Image.Image]) -> Tuple[Path, Path]:
    atlas, placements = pack_atlas(inputs)
    atlas_path = out_dir / "ui_atlas.png"
    atlas.save(atlas_path, "PNG")
    meta_path = out_dir / "ui_atlas_meta.json"
    meta = {
        "grid": 32,
        "sheet": atlas_path.name,
        "base_canvas": {"width": BASE_WIDTH, "height": BASE_HEIGHT},
        "nine_slice": {
            "corner_px": [SLICE_CORNER_MIN, SLICE_CORNER_MAX],
            "edge_px": [SLICE_EDGE_MIN, SLICE_EDGE_MAX],
        },
        "textures": {
            name: {
                "x": data["x"],
                "y": data["y"],
                "w": data["w"],
                "h": data["h"],
                "pad_w": data["pad_w"],
                "pad_h": data["pad_h"],
            }
            for name, data in placements.items()
        },
    }
    meta_path.write_text(json.dumps(meta, indent=2))
    return atlas_path, meta_path


def main() -> None:
    out_dir = ensure_out_dir()
    assets = dict([
        make_crt_overlay(),
        make_bezel(),
        make_window_chrome(),
        make_desk_texture(),
        make_conveyor_stripes(),
        make_timeline_icon("timeline_conveyor", paint_conveyor_icon),
        make_timeline_icon("timeline_box", paint_box_icon),
        make_timeline_icon("timeline_ready", paint_ready_icon),
        make_timeline_icon("timeline_ship", paint_ship_icon),
    ])
    atlas_path, meta_path = write_atlas(out_dir, assets)
    print(f"wrote {atlas_path}")
    print(f"wrote {meta_path}")


if __name__ == "__main__":
    main()
