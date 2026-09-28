// Logic tests for the rider physics and the rival AI, run in Node without a browser:
//   node web/qa/physics_test.mjs
import { register } from 'node:module';
register('./three_loader.mjs', import.meta.url);
const { Racer } = await import('../js/physics.js');
const { AIDriver } = await import('../js/ai.js');

let failed = 0;
const check = (name, ok, detail = '') => { console.log((ok ? 'PASS ' : 'FAIL ') + name + (detail ? '  ' + detail : '')); if (!ok) failed++; };
const dt = 1 / 60, idle = { steer: 0, tuck: false, brake: false, jump: false, grab: false, grabType: 0, boost: false };
const settled = () => { const r = new Racer({}); for (let i = 0; i < 120; i++) r.step(dt, idle); return r; };

check('rider settles on the ground', settled().grounded);

// Boost pushes on the frame it is pressed, and stops on the frame it is released.
{
  const a = settled(), b = settled(); a.boost = b.boost = 1;
  a.step(dt, { ...idle, boost: true }); b.step(dt, idle);
  check('boost acts on the frame it is pressed', a.speed > b.speed + 1e-6, `${a.speed.toFixed(4)} vs ${b.speed.toFixed(4)}`);
  check('boost drains the meter on the ground', a.boost < 1);
}
{
  const c = settled(), d = settled(); c.boost = d.boost = 1;
  c.step(dt, { ...idle, boost: true }); d.step(dt, { ...idle, boost: true });
  c.step(dt, idle); d.step(dt, { ...idle, boost: true });
  check('boost stops on the frame it is released', c.speed < d.speed, `${c.speed.toFixed(4)} vs ${d.speed.toFixed(4)}`);
}
{
  const e = settled(); e.grounded = false; e.boost = 1;
  e.step(dt, { ...idle, boost: true });
  check('boost does not drain in the air, where it cannot push', e.boost === 1 && !e.boosting);
}

// A rival that has finished stops racing and only brakes.
{
  const r = settled(), ai = new AIDriver(r, { seed: 3 });
  r.finished = true;
  const inp = ai.input(dt, 0, true);
  check('a finished rival gets no racing input', !inp.tuck && !inp.boost && !inp.jump && inp.steer === 0, JSON.stringify(inp));
}

console.log(failed ? `${failed} failed` : 'all passed');
process.exit(failed ? 1 : 0);
