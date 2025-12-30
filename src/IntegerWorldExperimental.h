#ifndef _INTEGER_WORLD_EXPERIMENTAL_INCLUDE_h
#define _INTEGER_WORLD_EXPERIMENTAL_INCLUDE_h

#include "IntegerWorld.h"

// Frustum debug helper with camera mesh and frustum edge drawing.
#include "Experimental/FrustumDebug.h"

// Monkey.
#include "Experimental/Suzanne.h"

// Animated demo scene with multiple objects and lights. Re-used for various cross-platform examples.
#include "Experimental/AnimatedDemoScene.h"

#if defined(ARDUINO)
// Pre made display engine with frame buffer, screen driver, render task, and optional FPS logging/drawing.
#include "Experimental/PremadeGraphicsDisplay.h"
#endif


#endif