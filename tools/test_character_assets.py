import unittest

from PIL import Image, ImageDraw
from build_character_assets import align_cells, check_border, cut_grid, faded, pixel_palette, remove_specks, sheet


class CharacterAssetsTest(unittest.TestCase):
    def test_grid_reading_order_and_nondivisible_dimensions(self):
        atlas = Image.new("RGBA", (101, 79))
        draw = ImageDraw.Draw(atlas)
        colors = ["red", "green", "blue", "yellow", "cyan", "magenta"]
        for index, color in enumerate(colors):
            x = round((index % 3 + 0.5) * atlas.width / 3)
            y = round((index // 3 + 0.5) * atlas.height / 2)
            draw.rectangle((x - 4, y - 6, x + 4, y + 6), fill=color)
        cells = cut_grid(atlas, 3, 2, pixel=True)
        self.assertEqual(len(cells), 6)
        self.assertEqual([cell.getpixel((4, 6)) for cell in cells],
                         [Image.new("RGBA", (1, 1), color).getpixel((0, 0)) for color in colors])

    def test_alignment_margin_palette_and_fade(self):
        cells = [Image.new("RGBA", size, color) for size, color in
                 [((13, 21), "purple"), ((17, 18), "navy"), ((11, 24), "green"), ((15, 22), "red")]]
        frames = pixel_palette(align_cells(cells, (32, 48), pixel=True))
        for frame in frames:
            check_border(frame)
            self.assertEqual(frame.size, (32, 48))
            self.assertEqual(frame.getchannel("A").getbbox()[3], 46)
        atlas = sheet(frames, 4)
        self.assertEqual(atlas.size, (128, 48))
        self.assertIsNone(faded(frames[0], 0).getchannel("A").getbbox())
        self.assertEqual(faded(frames[0], 255).tobytes(), frames[0].tobytes())

    def test_empty_cell_rejected(self):
        with self.assertRaises(ValueError):
            cut_grid(Image.new("RGBA", (64, 64)), 1, 1)

    def test_transparent_gutters_prevent_pose_clipping(self):
        atlas = Image.new("RGBA", (120, 40))
        draw = ImageDraw.Draw(atlas)
        for box, color in [((3, 3, 34, 36), "red"), ((37, 3, 83, 36), "green"),
                           ((88, 3, 117, 36), "blue")]:
            draw.rectangle(box, fill=color)
        cells = cut_grid(atlas, 3, 1, pixel=True)
        self.assertEqual([cell.size for cell in cells], [(32, 34), (47, 34), (30, 34)])

    def test_sheet_preserves_transparent_frame_bytes(self):
        frame = Image.new("RGBA", (32, 48), (20, 40, 60, 0))
        self.assertEqual(sheet([frame], 1).tobytes(), frame.tobytes())

    def test_specks_removed_but_detached_large_appendage_kept(self):
        image = Image.new("RGBA", (160, 160))
        draw = ImageDraw.Draw(image)
        draw.rectangle((40, 20, 100, 140), fill="navy")
        draw.rectangle((112, 80, 150, 130), fill="ivory")
        draw.rectangle((4, 4, 7, 7), fill="white")
        cleaned = remove_specks(image).getchannel("A")
        self.assertEqual(cleaned.getpixel((5, 5)), 0)
        self.assertEqual(cleaned.getpixel((70, 80)), 255)
        self.assertEqual(cleaned.getpixel((130, 100)), 255)

    def test_portrait_crop_excludes_neighbor_shoulder_at_cell_edge(self):
        atlas = Image.new("RGBA", (400, 600))
        draw = ImageDraw.Draw(atlas)
        draw.rectangle((35, 20, 360, 580), fill="navy")
        draw.rectangle((0, 300, 8, 580), fill="red")
        cells = cut_grid(atlas, 1, 1, portrait=True)
        self.assertNotIn((255, 0, 0, 255), [color for _, color in cells[0].getcolors()])


if __name__ == "__main__":
    unittest.main()
