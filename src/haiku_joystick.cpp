//========================================================================
// Copyright (c) 2021 Gerasim Troeglazov <3dEyes@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//========================================================================

extern "C" {
#include "internal.h"
}

#include <stdio.h>
#include <string.h>

#include <String.h>
#include <Joystick.h>

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
GLFWbool _glfwInitJoysticksHaiku(void)
{
	BJoystick *joystick = new BJoystick();

	if (joystick->CountDevices() <= 0)
		return GLFW_FALSE;

	char devName[B_OS_NAME_LENGTH];
	memset(devName, 0, B_OS_NAME_LENGTH);
	joystick->GetDeviceName(0, devName);
	if (joystick->Open(devName) == B_ERROR)
		return GLFW_FALSE;

	char name[B_OS_NAME_LENGTH];
	memset(name, 0, B_OS_NAME_LENGTH);
	strncpy(name, "Generic USB Joystick", sizeof(name));

	char guid[33] = "";
    sprintf(guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
		name[0], name[1], name[2], name[3],
		name[4], name[5], name[6], name[7],
		name[8], name[9], name[10]);

	_GLFWjoystick* js = _glfwAllocJoystick(name, guid,
		joystick->CountAxes(), joystick->CountButtons(), joystick->CountHats());

    js->haiku.device  = (void*)joystick;
    js->haiku.lastButtonValues = 0;

    _glfwInputJoystick(js, GLFW_CONNECTED);

	return GLFW_TRUE;
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticksHaiku(void)
{
	for (int jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++) {
		if (_glfw.joysticks[jid].present)
			delete (BJoystick*)(_glfw.joysticks[jid].haiku.device);
	}
}
//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformPollJoystick(_GLFWjoystick* js, int mode)
{
	BJoystick *joystick = (BJoystick*)js->haiku.device;
	joystick->Update();
	if (mode & _GLFW_POLL_AXES) {
		int16 axes[joystick->CountAxes()];
		joystick->GetAxisValues(axes);
		for (int axesIdx = 0; axesIdx < joystick->CountAxes(); axesIdx++) {
			double value = 0;
			if (axes[axesIdx] >= 0)
                value = axes[axesIdx] / 32767.0;
            else
                value = axes[axesIdx] / 32768.0;

			_glfwInputJoystickAxis(js, (int) axesIdx, value);
		}
	}

	if (mode & _GLFW_POLL_BUTTONS) {
		uint32 buttonValues = joystick->ButtonValues();
		for (int buttonIdx = 0; buttonIdx < joystick->CountButtons(); buttonIdx++) {
			bool buttonValue = buttonValues & (1 << buttonIdx);
			bool lastButtonValue = js->haiku.lastButtonValues & (1 << buttonIdx);
			if (buttonValue != lastButtonValue)
				_glfwInputJoystickButton(js, (int)buttonIdx, buttonValue ? GLFW_PRESS : GLFW_RELEASE);
		}
		js->haiku.lastButtonValues = buttonValues;

		int hatsCount = joystick->CountHats();
		uint8 hatsValues[hatsCount];
		joystick->GetHatValues(hatsValues, 0);

		for (int i = 0;  i < hatsCount;  i++) {
			const int states[9] = {
				GLFW_HAT_CENTERED,
				GLFW_HAT_UP,
				GLFW_HAT_RIGHT_UP,
				GLFW_HAT_RIGHT,
				GLFW_HAT_RIGHT_DOWN,
				GLFW_HAT_DOWN,
				GLFW_HAT_LEFT_DOWN,
				GLFW_HAT_LEFT,
				GLFW_HAT_LEFT_UP
			};

            long state = hatsValues[i];
			if (state < 0 || state > 8)
				state = 0;

			_glfwInputJoystickHat(js, (int)i, states[state]);
        }
	}
	return js->present;
}

void _glfwPlatformUpdateGamepadGUID(char* guid)
{
}

