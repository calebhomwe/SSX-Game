// Quality presets. The choice is remembered per browser and applied on reload.
export const PRESETS = {
  low: { name: 'Low', pixelRatio: 0.6, ds: 2.0, dv: 1.7, massifRes: 210, treesNear: 5000, treesFar: 5000, shadows: 1024, sparkle: false, particles: 700, boulders: 25, trailLen: 220 },
  medium: { name: 'Medium', pixelRatio: 0.85, ds: 1.25, dv: 1.1, massifRes: 300, treesNear: 11000, treesFar: 12000, shadows: 2048, sparkle: true, particles: 1400, boulders: 45, trailLen: 360 },
  high: { name: 'High', pixelRatio: 1.25, ds: 0.85, dv: 0.75, massifRes: 400, treesNear: 18000, treesFar: 22000, shadows: 2048, sparkle: true, particles: 2400, boulders: 70, trailLen: 520 },
};
const ORDER = ['low', 'medium', 'high'];

export function currentQualityKey() {
  const url = new URLSearchParams(location.search).get('q');
  if (url && PRESETS[url]) return url;
  try { const v = localStorage.getItem('summitline.quality'); if (v && PRESETS[v]) return v; } catch (e) { /* storage blocked */ }
  const coarse = matchMedia('(pointer: coarse)').matches;
  return coarse ? 'low' : 'medium';
}
export function nextQualityKey(k) { return ORDER[(ORDER.indexOf(k) + 1) % ORDER.length]; }
export function saveQualityKey(k) { try { localStorage.setItem('summitline.quality', k); } catch (e) { /* ignore */ } }
