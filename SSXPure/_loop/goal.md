# Goal: SSX Tricky Fidelity - Pure C++ UE 5.8 Snowboarding Game

Build an SSX-style arcade snowboarding game in Unreal Engine 5.8 using ONLY C++.
Zero assets, zero blueprints, zero marketplace content. Every mesh, material,
sound, particle, and texture is generated procedurally in code at runtime.

## Acceptance criteria (Phase 1 - CORE)
- [ ] Project builds clean (SSXPureEditor Win64 Development)
- [ ] Infinite procedural Perlin terrain with ramps/half-pipes
- [ ] Physics snowboarder (board/body/head box-built) with carve, jump, slope gravity
- [ ] Trick system: spin/flip detection, scoring, combo multiplier, trick names
- [ ] Canvas HUD: speed, score, trick pop-ups
- [ ] Snow spray particles on carve
- [ ] Dynamic follow camera
- [ ] Headless verification: -ShotTest captures a real-RHI screenshot

## Acceptance criteria (Phase 2 - IDENTITY)
- [ ] Named trick dictionary (Indy, Melon, Method, Tailgrab, 180-1080, Back/Flip, Rodeo, Cork)
- [ ] Procedural audio synth (wind by speed, board scrape, jump whoosh)
- [ ] Camera FOV warp with speed, screen shake, HUD juice (SSX3-style)
- [ ] Biome variation (alpine/glacier colors) + procedural props (trees/rocks)
- [ ] Game flow: start gate, finish/goal, restart

## Roadmap source
Docs: chat-export-1786058808480.json + "Pure C++ Game Development ssx.docx"
(Kimi conversation: SSXPure phase 1 code + 6-phase roadmap to Steam release)
