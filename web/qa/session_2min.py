"""Real-time 2-minute session: no timelapse, scripted input, 1280x800.
Logs every console error, page error and failed request, and samples frame times.
Usage: python3 session_2min.py <out_dir> [quality]
"""
import asyncio, json, sys, time
from playwright.async_api import async_playwright
URL = 'http://127.0.0.1:8765/SSX-Game/web/index.html'
CHROME = '/opt/pw-browsers/chromium-1194/chrome-linux/chrome'


async def main(out, q):
    async with async_playwright() as p:
        b = await p.chromium.launch(executable_path=CHROME, args=['--no-sandbox', '--use-gl=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist', '--autoplay-policy=no-user-gesture-required'])
        pg = await b.new_page(viewport={'width': 1280, 'height': 800})
        log = []
        pg.on('console', lambda m: log.append(f'{m.type}: {m.text[:300]}'))
        pg.on('pageerror', lambda e: log.append(f'PAGEERROR: {str(e)[:300]}'))
        pg.on('response', lambda r: log.append(f'HTTP {r.status} {r.url}') if r.status >= 400 else None)
        pg.on('requestfailed', lambda r: log.append(f'REQUESTFAILED {r.url}'))
        t0 = time.time()
        await pg.goto(f'{URL}?q={q}&autoplay=1', timeout=300000)
        await pg.wait_for_function('window.__game && __game.state() === "title"', timeout=900000, polling=500)
        load = time.time() - t0
        await pg.evaluate('window.__frameTimes.length = 0; document.querySelector("#btnPlay").click()')
        samples = []
        t1 = time.time()
        while time.time() - t1 < 120:
            await pg.wait_for_timeout(10000)
            samples.append(json.loads(await pg.evaluate('JSON.stringify({state: __game.state(), s: Math.round(__game.player().s), kmh: Math.round(__game.player().speed*3.6), frames: window.__frames})')))
        ft = await pg.evaluate('[...window.__frameTimes]')
        await pg.screenshot(path=f'{out}/session_2min_end.png', timeout=240000)
        s = sorted(ft)
        errs = [l for l in log if l.startswith(('error', 'PAGEERROR', 'HTTP', 'REQUESTFAILED'))]
        rep = {'quality': q, 'viewport': '1280x800', 'load_to_title_s': round(load, 1), 'session_s': round(time.time() - t1, 1),
               'frames': len(ft), 'median_frame_ms': round(s[len(s) // 2], 1) if s else None,
               'p90_frame_ms': round(s[int(len(s) * 0.9)], 1) if s else None, 'worst_frame_ms': round(s[-1], 1) if s else None,
               'samples': samples, 'errors': errs, 'console': [l for l in log if 'GPU stall' not in l]}
        json.dump(rep, open(f'{out}/session_2min.json', 'w'), indent=1)
        print(json.dumps({k: v for k, v in rep.items() if k != 'console'}, indent=1))
        await b.close()

asyncio.run(main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else 'low'))
