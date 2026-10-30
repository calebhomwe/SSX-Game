#include "../Source/SSXPure/SSXPureContact.h"
#include <cassert>

int main()
{
	assert(!IsBoardGrounded(false, 0.f, false, 0.f));
	assert(!IsBoardGrounded(true, 150.f, false, 0.f)); // visible slope, still in air
	assert(IsBoardGrounded(true, 58.f, false, 0.f));
	assert(!IsBoardGrounded(true, 58.f, true, 200.f)); // leaving the slope
	assert(IsBoardGrounded(true, 58.f, true, -200.f)); // landing
	assert(IsBoardGrounded(true, 58.f, true, 0.f)); // comes to rest near the slope
}
