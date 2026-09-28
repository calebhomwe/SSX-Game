"""Input smoke test: real keyboard events on desktop and real touch taps on a
phone viewport (no autoplay). Checks the rider responds and nothing errors.

Usage: python3 input_smoke.py <out_dir>
"""
import asyncio, json, sys
from playwright.async_api import async_playwright

URL = 'http://127.0.0.1:8765/SSX-Game/web/index.html?q=low'
CHROME = '/opt/pw-browsers/chromium-1194/chrome-linux/chrome'
STATE = 'JSON.stringify((p => ({state: __game.state(), s: +p.s.toFixed(1), kmh: Math.round(p.speed * 3.6), yaw: +p.yaw.toFixed(2), grounded: p.grounded, score: p.score, boost: +p.boost.toFixed(2)}))(__game.player()))'


async def frames(pg, n):
    f = await pg.evaluate('window.__frames || 0')
    await pg.wait_for_function(f'(window.__frames || 0) >= {f + n}', timeout=1800000, polling=200)


async def run(p, out, mode):
    mob = mode == 'phone'
    b = await p.chromium.launch(executable_path=CHROME, args=['--no-sandbox', '--use-gl=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'])
    ctx = await b.new_context(viewport={'width': 390, 'height': 844} if mob else {'width': 960, 'height': 600}, is_mobile=mob, has_touch=mob)
    pg = await ctx.new_page()
    log, steps = [], []
    pg.on('console', lambda m: log.append(f'{m.type}: {m.text[:200]}'))
    pg.on('pageerror', lambda e: log.append(f'PAGEERROR: {e}'))
    pg.on('response', lambda r: log.append(f'HTTP {r.status} {r.url}') if r.status >= 400 else None)
    await pg.goto(URL, timeout=300000)
    await pg.wait_for_function('window.__game && __game.state() === "title"', timeout=900000, polling=500)
    await pg.evaluate('__game.timelapse(12)')
    if mob:
        await pg.tap('#btnPlay')
    else:
        await pg.keyboard.press('Enter')
    await pg.wait_for_function('__game.state() === "race"', timeout=900000, polling=200)
    steps.append(['start', json.loads(await pg.evaluate(STATE))])
    if mob:
        box = await pg.locator('#stick').bounding_box()
        cx, cy = box['x'] + box['width'] / 2, box['y'] + box['height'] / 2
        cdp = await ctx.new_cdp_session(pg)
        async def touch(kind, pts):
            await cdp.send('Input.dispatchTouchEvent', {'type': kind, 'touchPoints': [{'x': x, 'y': y, 'id': i} for i, (x, y) in enumerate(pts)]})
        await touch('touchStart', [(cx, cy)]); await touch('touchMove', [(cx + 50, cy)])
        await frames(pg, 4); steps.append(['stick right', json.loads(await pg.evaluate(STATE))])
        await touch('touchMove', [(cx - 50, cy)])
        await frames(pg, 4); steps.append(['stick left', json.loads(await pg.evaluate(STATE))])
        await touch('touchEnd', [])
        jb = await pg.locator('#tJump').bounding_box()
        await touch('touchStart', [(jb['x'] + 40, jb['y'] + 40)]); await frames(pg, 2); await touch('touchEnd', [])
        await frames(pg, 1); steps.append(['jump tap', json.loads(await pg.evaluate(STATE))])
    else:
        for key, n in (('KeyW', 4), ('KeyD', 3), ('KeyA', 3), ('ShiftLeft', 3)):
            await pg.keyboard.down(key); await frames(pg, n); await pg.keyboard.up(key)
            steps.append([key, json.loads(await pg.evaluate(STATE))])
        await pg.keyboard.down('Space'); await frames(pg, 2); await pg.keyboard.up('Space'); await frames(pg, 1)
        steps.append(['Space (ollie)', json.loads(await pg.evaluate(STATE))])
        await pg.keyboard.press('Escape'); await frames(pg, 1)
        steps.append(['Escape', {'pause_visible': await pg.evaluate('document.querySelector("#pause").classList.contains("show")')}])
        await pg.keyboard.press('Escape'); await frames(pg, 1)
    await pg.screenshot(path=f'{out}/input_{mode}.png', timeout=240000)
    errs = [l for l in log if l.startswith(('error', 'PAGEERROR', 'HTTP'))]
    await b.close()
    return {'mode': mode, 'steps': steps, 'errors': errs}


async def main(out):
    async with async_playwright() as p:
        res = [await run(p, out, 'desk'), await run(p, out, 'phone')]
    json.dump(res, open(f'{out}/input_smoke.json', 'w'), indent=1)
    print(json.dumps(res, indent=1))

asyncio.run(main(sys.argv[1]))
