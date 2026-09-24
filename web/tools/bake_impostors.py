"""Offline impostor baker for the Poly Haven fir_tree_01 model.

Usage: python3 bake_impostors.py <out_dir> <model path under .bake/> <prefix>
Needs the static server on :8765 and the model at SSX-Game/.bake/fir/fir_tree_01.gltf
Writes fir_albedo.webp (RGBA) and fir_normal.webp (RGB) atlases: 8 views x 3 variants.
"""
import asyncio, base64, io, json, sys
import numpy as np
from PIL import Image, ImageFilter
from playwright.async_api import async_playwright

NV = 8
CELL = (256, 512)
URL = "http://127.0.0.1:8765/SSX-Game/web/tools/bake_impostors.html?src=../../.bake/"


def dilate(rgb, a, iters=24):
    rgb = rgb.astype(np.float32)
    m = (a > 8).astype(np.float32)
    acc = rgb * m[..., None]
    for _ in range(iters):
        if m.min() > 0:
            break
        pad = np.pad(acc, ((1, 1), (1, 1), (0, 0)))
        pm = np.pad(m, 1)
        s = sum(pad[1 + dy:pad.shape[0] - 1 + dy, 1 + dx:pad.shape[1] - 1 + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1))
        c = sum(pm[1 + dy:pm.shape[0] - 1 + dy, 1 + dx:pm.shape[1] - 1 + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1))
        new = (m == 0) & (c > 0)
        acc[new] = s[new] / c[new][:, None]
        m = np.maximum(m, new.astype(np.float32))
    return acc


async def main(out, src, prefix):
    async with async_playwright() as p:
        b = await p.chromium.launch(executable_path='/opt/pw-browsers/chromium-1194/chrome-linux/chrome',
                                    args=['--no-sandbox', '--use-gl=swiftshader', '--enable-unsafe-swiftshader'])
        pg = await b.new_page(viewport={'width': 600, 'height': 1100})
        pg.on('console', lambda m: print('console', m.text))
        await pg.goto(URL + src, timeout=600000)
        await pg.wait_for_function('window.bakeInfo !== null', timeout=900000, polling=2000)
        info = await pg.evaluate('window.bakeInfo')
        print(info)
        nt = len(info)
        alb = Image.new('RGBA', (CELL[0] * NV, CELL[1] * nt))
        nor = Image.new('RGB', (CELL[0] * NV, CELL[1] * nt))
        meta = []
        for ti in range(nt):
            for v in range(NV):
                cells = {}
                for pas in ('albedo', 'normal'):
                    r = await pg.evaluate(f'window.bake({ti},{v},{NV},"{pas}")')
                    im = Image.open(io.BytesIO(base64.b64decode(r['url'].split(',')[1]))).convert('RGBA')
                    cells[pas] = im
                    if v == 0:
                        meta.append({'halfW': r['halfW'], 'halfH': r['halfH']})
                a_full = np.asarray(cells['albedo'])
                alpha = a_full[..., 3]
                # downsample 2x with premultiplied colour
                A = Image.fromarray(alpha).resize(CELL, Image.LANCZOS)
                aa = np.asarray(A)
                col = dilate(a_full[..., :3], alpha)
                C = Image.fromarray(np.clip(col, 0, 255).astype(np.uint8)).resize(CELL, Image.LANCZOS)
                ncol = dilate(np.asarray(cells['normal'])[..., :3], alpha)
                N = Image.fromarray(np.clip(ncol, 0, 255).astype(np.uint8)).resize(CELL, Image.LANCZOS)
                rgba = Image.merge('RGBA', (*C.split(), Image.fromarray(aa)))
                alb.paste(rgba, (v * CELL[0], ti * CELL[1]))
                nor.paste(N, (v * CELL[0], ti * CELL[1]))
                print('tree', ti, 'view', v, flush=True)
        alb.save(f'{out}/{prefix}_albedo.webp', quality=90, method=6)
        nor.save(f'{out}/{prefix}_normal.webp', quality=88, method=6)
        with open(f'{out}/{prefix}_meta.json', 'w') as f:
            json.dump({'views': NV, 'cell': CELL, 'trees': [dict(i, **m) for i, m in zip(info, meta)]}, f, indent=1)
        await b.close()

asyncio.run(main(sys.argv[1], sys.argv[2], sys.argv[3]))
