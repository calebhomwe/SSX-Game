"""Build the side-by-side comparison sheet (our capture | reference).

The reference screenshots are publisher images from the Steam store pages of
Riders Republic and Steep. They are used for internal comparison only, so the
refs folder and the generated sheet are git-ignored and never shipped.

Usage: python3 make_sheet.py <captures_dir> <refs_dir> <out.jpg>
"""
import sys
from PIL import Image, ImageDraw, ImageFont

PAIRS = [
    ('desk_03_midrun.png', '2290180_2.jpg', 'Mid-run carve', 'Riders Republic (Steam store screenshot)'),
    ('desk_04_midair.png', '2290180_5.jpg', 'Mid-air grab', 'Riders Republic (Steam store screenshot)'),
    ('desk_01_title.png', '460920_8.jpg', 'Title / vista', 'Steep (Steam store screenshot)'),
]
W, H = 800, 450


def main(cap, refs, out):
    font = ImageFont.load_default(size=22) if hasattr(ImageFont, 'load_default') else None
    sheet = Image.new('RGB', (W * 2 + 30, (H + 44) * len(PAIRS) + 10), (14, 22, 36))
    d = ImageDraw.Draw(sheet)
    for i, (a, b, la, lb) in enumerate(PAIRS):
        y = 10 + i * (H + 44)
        ia = Image.open(f'{cap}/{a}').convert('RGB')
        ia = ia.crop((0, (ia.height - ia.width * 9 // 16) // 2, ia.width, (ia.height + ia.width * 9 // 16) // 2)).resize((W, H))
        ib = Image.open(f'{refs}/{b}').convert('RGB').resize((W, H))
        sheet.paste(ia, (10, y + 34)); sheet.paste(ib, (W + 20, y + 34))
        d.text((12, y + 4), f'SUMMIT LINE - {la}', fill=(255, 200, 80), font=font)
        d.text((W + 22, y + 4), f'REFERENCE - {lb}', fill=(180, 200, 230), font=font)
    sheet.save(out, quality=88)


main(*sys.argv[1:4])
