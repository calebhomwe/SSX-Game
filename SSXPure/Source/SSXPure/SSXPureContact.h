#pragma once

// A downward probe can see the slope long before the board actually reaches it.
// Once airborne, require the board to stop moving away from the surface as well.
inline bool IsBoardGrounded(bool hasHit, float distance, bool wasAirborne, float normalSpeed)
{
	return hasHit && distance < 90.f && (!wasAirborne || normalSpeed <= 10.f);
}
