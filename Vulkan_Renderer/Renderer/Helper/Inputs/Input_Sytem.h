#pragma once

#include "KeyCodes.h"

#pragma once

namespace tde
{
	// Declare the struct but without `static` so it can be defined once elsewhere
	struct Keys
	{
		bool forward = false;
		bool backward = false;
		bool up = false;
		bool down = false;
		bool right = false;
		bool left = false;
		bool wireframe = false;

		//Look
		bool rot_up = false;
		bool rot_down = false;
		bool rot_right = false;
		bool rot_left = false;
	};

	// Declare a global instance of Keys
	extern Keys keys;

	// moving() function checks if any key is pressed
	inline bool moving()
	{
		return keys.up || keys.down || keys.right || keys.left || keys.forward || keys.backward;
	}

	inline bool rotating()
	{
		return keys.rot_down || keys.rot_left || keys.rot_right || keys.rot_up;
	}
}