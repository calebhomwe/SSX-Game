// Lets Node import the game's browser modules: resolves the bare 'three' import the way index.html's import map does.
export async function resolve(spec, ctx, next) {
  if (spec === 'three') return next(new URL('../vendor/three/build/three.module.min.js', import.meta.url).href, ctx);
  return next(spec, ctx);
}
