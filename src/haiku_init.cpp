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

#include <AppKit.h>
#include <MessageQueue.h>
#include <storage/Path.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <storage/AppFileInfo.h>

#include <Rect.h>
#include <Screen.h>

// Default application signature
const char *signature = "application/x-vnd.GLFW-executable";

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_glfw.haiku.keycodes, -1, sizeof(_glfw.haiku.keycodes));
    memset(_glfw.haiku.scancodes, -1, sizeof(_glfw.haiku.scancodes));

    _glfw.haiku.keycodes[0x01] = GLFW_KEY_ESCAPE;
    _glfw.haiku.keycodes[0x02] = GLFW_KEY_F1;
    _glfw.haiku.keycodes[0x03] = GLFW_KEY_F2;
    _glfw.haiku.keycodes[0x04] = GLFW_KEY_F3;
    _glfw.haiku.keycodes[0x05] = GLFW_KEY_F4;
    _glfw.haiku.keycodes[0x06] = GLFW_KEY_F5;
    _glfw.haiku.keycodes[0x07] = GLFW_KEY_F6;
    _glfw.haiku.keycodes[0x08] = GLFW_KEY_F7;
    _glfw.haiku.keycodes[0x09] = GLFW_KEY_F8;
    _glfw.haiku.keycodes[0x0A] = GLFW_KEY_F9;
    _glfw.haiku.keycodes[0x0B] = GLFW_KEY_F10;
    _glfw.haiku.keycodes[0x0C] = GLFW_KEY_F11;
    _glfw.haiku.keycodes[0x0D] = GLFW_KEY_F12;
	_glfw.haiku.keycodes[0x0E] = GLFW_KEY_PRINT_SCREEN;
    _glfw.haiku.keycodes[0x0F] = GLFW_KEY_SCROLL_LOCK;
    _glfw.haiku.keycodes[0x10] = GLFW_KEY_PAUSE;

    _glfw.haiku.keycodes[0x11] = GLFW_KEY_GRAVE_ACCENT;
    _glfw.haiku.keycodes[0x12] = GLFW_KEY_1;
    _glfw.haiku.keycodes[0x13] = GLFW_KEY_2;
    _glfw.haiku.keycodes[0x14] = GLFW_KEY_3;
    _glfw.haiku.keycodes[0x15] = GLFW_KEY_4;
    _glfw.haiku.keycodes[0x16] = GLFW_KEY_5;
    _glfw.haiku.keycodes[0x17] = GLFW_KEY_6;
    _glfw.haiku.keycodes[0x18] = GLFW_KEY_7;
    _glfw.haiku.keycodes[0x19] = GLFW_KEY_8;
    _glfw.haiku.keycodes[0x1A] = GLFW_KEY_9;
    _glfw.haiku.keycodes[0x1B] = GLFW_KEY_0;
    _glfw.haiku.keycodes[0x1C] = GLFW_KEY_MINUS;
    _glfw.haiku.keycodes[0x1D] = GLFW_KEY_EQUAL;
    _glfw.haiku.keycodes[0x1E] = GLFW_KEY_BACKSPACE;
    _glfw.haiku.keycodes[0x1F] = GLFW_KEY_INSERT;
    _glfw.haiku.keycodes[0x20] = GLFW_KEY_HOME;
    _glfw.haiku.keycodes[0x21] = GLFW_KEY_PAGE_UP;
    _glfw.haiku.keycodes[0x22] = GLFW_KEY_NUM_LOCK;
    _glfw.haiku.keycodes[0x23] = GLFW_KEY_KP_DIVIDE;
    _glfw.haiku.keycodes[0x24] = GLFW_KEY_KP_MULTIPLY;
    _glfw.haiku.keycodes[0x25] = GLFW_KEY_KP_SUBTRACT;

    _glfw.haiku.keycodes[0x26] = GLFW_KEY_TAB;
    _glfw.haiku.keycodes[0x27] = GLFW_KEY_Q;
    _glfw.haiku.keycodes[0x28] = GLFW_KEY_W;
    _glfw.haiku.keycodes[0x29] = GLFW_KEY_E;
    _glfw.haiku.keycodes[0x2A] = GLFW_KEY_R;
    _glfw.haiku.keycodes[0x2B] = GLFW_KEY_T;
    _glfw.haiku.keycodes[0x2C] = GLFW_KEY_Y;
    _glfw.haiku.keycodes[0x2D] = GLFW_KEY_U;
    _glfw.haiku.keycodes[0x2E] = GLFW_KEY_I;
    _glfw.haiku.keycodes[0x2F] = GLFW_KEY_O;
    _glfw.haiku.keycodes[0x30] = GLFW_KEY_P;
    _glfw.haiku.keycodes[0x31] = GLFW_KEY_LEFT_BRACKET;
    _glfw.haiku.keycodes[0x32] = GLFW_KEY_RIGHT_BRACKET;
    _glfw.haiku.keycodes[0x33] = GLFW_KEY_BACKSLASH;
    _glfw.haiku.keycodes[0x34] = GLFW_KEY_DELETE;
    _glfw.haiku.keycodes[0x35] = GLFW_KEY_END;
    _glfw.haiku.keycodes[0x36] = GLFW_KEY_PAGE_DOWN;
    _glfw.haiku.keycodes[0x37] = GLFW_KEY_KP_7;
    _glfw.haiku.keycodes[0x38] = GLFW_KEY_KP_8;
    _glfw.haiku.keycodes[0x39] = GLFW_KEY_KP_9;
    _glfw.haiku.keycodes[0x3A] = GLFW_KEY_KP_ADD;

    _glfw.haiku.keycodes[0x3B] = GLFW_KEY_CAPS_LOCK;
    _glfw.haiku.keycodes[0x3C] = GLFW_KEY_A;
    _glfw.haiku.keycodes[0x3D] = GLFW_KEY_S;
    _glfw.haiku.keycodes[0x3E] = GLFW_KEY_D;
    _glfw.haiku.keycodes[0x3F] = GLFW_KEY_F;
    _glfw.haiku.keycodes[0x40] = GLFW_KEY_G;
    _glfw.haiku.keycodes[0x41] = GLFW_KEY_H;
    _glfw.haiku.keycodes[0x42] = GLFW_KEY_J;
    _glfw.haiku.keycodes[0x43] = GLFW_KEY_K;
    _glfw.haiku.keycodes[0x44] = GLFW_KEY_L;
    _glfw.haiku.keycodes[0x45] = GLFW_KEY_SEMICOLON;
    _glfw.haiku.keycodes[0x46] = GLFW_KEY_APOSTROPHE;
    _glfw.haiku.keycodes[0x47] = GLFW_KEY_ENTER;
    _glfw.haiku.keycodes[0x48] = GLFW_KEY_KP_4;
    _glfw.haiku.keycodes[0x49] = GLFW_KEY_KP_5;
    _glfw.haiku.keycodes[0x4A] = GLFW_KEY_KP_6;

    _glfw.haiku.keycodes[0x4B] = GLFW_KEY_LEFT_SHIFT;
    _glfw.haiku.keycodes[0x4C] = GLFW_KEY_Z;
    _glfw.haiku.keycodes[0x4D] = GLFW_KEY_X;
    _glfw.haiku.keycodes[0x4E] = GLFW_KEY_C;
    _glfw.haiku.keycodes[0x4F] = GLFW_KEY_V;
    _glfw.haiku.keycodes[0x50] = GLFW_KEY_B;
    _glfw.haiku.keycodes[0x51] = GLFW_KEY_N;
    _glfw.haiku.keycodes[0x52] = GLFW_KEY_M;
    _glfw.haiku.keycodes[0x53] = GLFW_KEY_COMMA;
    _glfw.haiku.keycodes[0x54] = GLFW_KEY_PERIOD;
    _glfw.haiku.keycodes[0x55] = GLFW_KEY_SLASH;
    _glfw.haiku.keycodes[0x56] = GLFW_KEY_RIGHT_SHIFT;
    _glfw.haiku.keycodes[0x57] = GLFW_KEY_UP;
    _glfw.haiku.keycodes[0x58] = GLFW_KEY_KP_1;
    _glfw.haiku.keycodes[0x59] = GLFW_KEY_KP_2;
    _glfw.haiku.keycodes[0x5A] = GLFW_KEY_KP_3;
    _glfw.haiku.keycodes[0x5B] = GLFW_KEY_KP_ENTER;

    _glfw.haiku.keycodes[0x5C] = GLFW_KEY_LEFT_CONTROL;
    _glfw.haiku.keycodes[0x5D] = GLFW_KEY_LEFT_ALT;
    _glfw.haiku.keycodes[0x5E] = GLFW_KEY_SPACE;
    _glfw.haiku.keycodes[0x5F] = GLFW_KEY_RIGHT_ALT;
    _glfw.haiku.keycodes[0x60] = GLFW_KEY_RIGHT_CONTROL;
    _glfw.haiku.keycodes[0x61] = GLFW_KEY_LEFT;
    _glfw.haiku.keycodes[0x62] = GLFW_KEY_DOWN;
    _glfw.haiku.keycodes[0x63] = GLFW_KEY_RIGHT;
    _glfw.haiku.keycodes[0x64] = GLFW_KEY_KP_0;
    _glfw.haiku.keycodes[0x65] = GLFW_KEY_KP_DECIMAL;
    _glfw.haiku.keycodes[0x66] = GLFW_KEY_LEFT_SUPER;
    _glfw.haiku.keycodes[0x67] = GLFW_KEY_RIGHT_SUPER;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        if (_glfw.haiku.keycodes[scancode] >= 0)
            _glfw.haiku.scancodes[_glfw.haiku.keycodes[scancode]] = scancode;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
	if (!be_app) {
	    image_info info;
	    int32 cookie = 0;
	    if (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
	        BFile file(info.name, O_RDONLY);
	        if (file.InitCheck() == B_OK) {
	            BAppFileInfo app_info(&file);
	            if (app_info.InitCheck() == B_OK) {
	                char sig[B_MIME_TYPE_LENGTH];
	                if (app_info.GetSignature(sig) == B_OK)
	                    signature = strndup(sig, B_MIME_TYPE_LENGTH);
	            }
	        }
	    }
		be_app = new BApplication(signature);
	}

	createKeyTables();

	_glfw.haiku.messageQueue = (void*)new BMessageQueue();

	_glfwInitTimerPOSIX();
	_glfwInitJoysticksHaiku();
	
	BScreen scr(B_MAIN_SCREEN_ID);

	float dpi = 90.0;
	float widthMM = (scr.Frame().Width() / dpi) * 2.54;
	float heightMM = (scr.Frame().Height() / dpi) * 2.54;

    _GLFWmonitor *monitor = _glfwAllocMonitor("Default monitor", widthMM, heightMM);
    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_LAST);
    
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
	delete (BMessageQueue*)_glfw.haiku.messageQueue;
    _glfwTerminateOSMesa();
    _glfwTerminateJoysticksHaiku();
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER " Haiku OSMesa";
}
