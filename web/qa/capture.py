"""Evidence capture for the quality gate.

Drives the real game page (index.html) in Chromium/SwiftShader with the QA hooks:
  ?autoplay=1        scripted rider input (same controller the rivals use)
  __game.timelapse(n) run n physics steps (1/60 s each) per rendered frame
  __game.hold(bool)   freeze the simulation while a still is taken

Usage: python3 capture.py <out_dir> desk|phone [quality]
"""
import asyncio, json, sys, time
from playwright.async_api import async_playwright

URL = 'http://127.0.0.1:8765/SSX-Game/web/index.html'
CHROME = '/opt/pw-browsers/chromium-1194/chrome-linux/chrome'


async def main(out, mode, q):
    vp = {'width': 1280, 'height': 800} if mode == 'desk' else {'width': 390, 'height': 844}
    mob = mode == 'phone'
    async with async_playwright() as p:
        b = await p.chromium.launch(executable_path=CHROME, args=['--no-sandbox', '--use-gl=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist', '--autoplay-policy=no-user-gesture-required'])
        ctx = await b.new_context(viewport=vp, is_mobile=mob, has_touch=mob, device_scale_factor=1)
        pg = await ctx.new_page()
        log = []
        pg.on('console', lambda m: log.append(f'{m.type}: {m.text[:300]}'))
        pg.on('pageerror', lambda e: log.append(f'PAGEERROR: {str(e)[:300]}'))
        pg.on('response', lambda r: log.append(f'HTTP {r.status} {r.url}') if r.status >= 400 else None)
        pg.on('requestfailed', lambda r: log.append(f'REQUESTFAILED {r.url}'))
        t0 = time.time()

        async def shot(name):
            await pg.screenshot(path=f'{out}/{mode}_{name}.png', timeout=240000)
            print(f'{time.time() - t0:7.1f}s  {mode}_{name}.png', flush=True)

        async def until(js, timeout=3600000):
            await pg.wait_for_function(js, timeout=timeout, polling=250)

        async def frames(n):
            f = await pg.evaluate('window.__frames || 0')
            await until(f'(window.__frames || 0) >= {f + n}')

        await pg.goto(f'{URL}?q={q}&autoplay=1', timeout=300000)
        await until('window.__game && __game.state() === "title"')
        await pg.wait_for_timeout(1500)
        await shot('01_title')
        await pg.evaluate('document.querySelector("#btnControls").click()')
        await frames(1)
        await shot('02_controls')
        await pg.evaluate('document.querySelector("#helpClose").click()')
        await pg.evaluate('__game.timelapse(90); document.querySelector("#btnPlay").click()')
        # mid-run: carving at speed in the top section
        await until('__game.player().s > 130')
        await pg.evaluate('__game.timelapse(3)')
        await frames(3)
        await pg.evaluate('__game.hold(true)')
        await shot('03_midrun')
        await pg.evaluate('__game.hold(false); __game.timelapse(40)')
        # mid-air off the first kicker
        await until('__game.player().s > 270')
        await pg.evaluate('__game.timelapse(8)')
        await until('!__game.player().grounded && __game.player().airTime > 0.5 && __game.height() > 2.2')
        await pg.evaluate('__game.hold(true)')
        await shot('04_midair')
        await pg.evaluate('__game.hold(false); __game.timelapse(10)')
        # reward: trick banner right after the landing
        await until('__game.player().grounded && __game.player().score > 0')
        await pg.evaluate('__game.hold(true)')
        await shot('05_trick')
        await pg.evaluate('__game.hold(false)')
        # pause menu
        await pg.evaluate('document.querySelector("#btnPause").click()')
        await frames(1)
        await shot('06_pause')
        await pg.evaluate('document.querySelector("#btnResume").click(); __game.timelapse(600)')
        await until('__game.state() === "finish" || __game.state() === "results"')
        await pg.evaluate('__game.timelapse(30)')
        await until('__game.state() === "results"')
        await frames(2)
        await shot('07_results')
        stats = await pg.evaluate('''(() => { const p = __game.player(); const ft = [...window.__frameTimes].sort((a,b)=>a-b);
            return { time: p.finishTime, score: p.score, best: p.bestTrick, air: p.totalAir, top: p.topSpeed * 3.6,
                     frames: ft.length, median_ms: ft[Math.floor(ft.length/2)], worst_ms: ft[ft.length-1],
                     rivals: __game.racers().map(r => [r.name, r.finishTime]) }; })()''')
        wall = time.time() - t0
        errs = [l for l in log if l.startswith(('error', 'PAGEERROR', 'HTTP', 'REQUESTFAILED'))]
        rep = {'mode': mode, 'quality': q, 'wall_seconds': round(wall, 1), 'stats': stats, 'errors': errs, 'console': [l for l in log if 'GPU stall' not in l]}
        json.dump(rep, open(f'{out}/{mode}_session.json', 'w'), indent=1)
        print(json.dumps({k: rep[k] for k in ('mode', 'wall_seconds', 'stats', 'errors')}, indent=1))
        await b.close()

asyncio.run(main(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else 'medium'))
