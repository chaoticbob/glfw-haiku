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

#include <Rect.h>
#include <Screen.h>
#include <stdio.h>

static void addVideoMode(_GLFWmonitor *monitor, int32_t width, int32_t height, float refresh)
{
    GLFWvidmode mode;

    mode.width = width;
    mode.height = height;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = (int)refresh;

	if (monitor->modeCount > 0 && monitor->modes) {
		for (int i = 0; i < monitor->modeCount; i++) {
			if (monitor->modes[i].width == width && 
				monitor->modes[i].height == height &&
				monitor->modes[i].refreshRate == refresh)
				return;
		}
	}

    monitor->modeCount++;
    if (monitor->modes)
    	monitor->modes = (GLFWvidmode*)realloc(monitor->modes, monitor->modeCount * sizeof(GLFWvidmode));
    else
    	monitor->modes = (GLFWvidmode*)malloc(monitor->modeCount * sizeof(GLFWvidmode));

    monitor->modes[monitor->modeCount - 1] = mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformFreeMonitor(_GLFWmonitor* monitor)
{
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
	if (xpos)
	   *xpos = 0;
	if (ypos)
	   *ypos = 0;
}

void _glfwPlatformGetMonitorContentScale(_GLFWmonitor* monitor,
                                         float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwPlatformGetMonitorWorkarea(_GLFWmonitor* monitor,
                                     int* xpos, int* ypos,
                                     int* width, int* height)
{
	if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = monitor->modes[0].width;
    if (height)
        *height = monitor->modes[0].height;
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
	BScreen scr(B_MAIN_SCREEN_ID);

	if (!monitor->modes) {
		display_mode hmode;
	    scr.GetMode(&hmode);
	    float refresh = float(hmode.timing.pixel_clock * 1000) / float(hmode.timing.h_total * hmode.timing.v_total);
#if 0
		addVideoMode(monitor, 640, 480, refresh);
		addVideoMode(monitor, 800, 600, refresh);
		addVideoMode(monitor, 1024, 768, refresh);
#endif
		addVideoMode(monitor, hmode.virtual_width, hmode.virtual_height, refresh);
	}

	*found = monitor->modeCount;
	return monitor->modes;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* _mode)
{
	BScreen scr(B_MAIN_SCREEN_ID);
	GLFWvidmode mode;
	mode.width = scr.Frame().Width() + 1;
    mode.height = scr.Frame().Height() + 1;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = 60;

 	*_mode = mode;
}

GLFWbool _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    return GLFW_FALSE;
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
}

