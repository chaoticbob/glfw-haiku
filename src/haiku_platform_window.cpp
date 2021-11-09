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

#include "haiku_platform_window.h"

#include <stdlib.h>
#include <iostream>

HaikuPlatformWindow::HaikuPlatformWindow(BRect frame, const char* title, window_type type, uint32 flags, _GLFWwindow* glwin)
	: BWindow(frame, title, type, flags)
{
	window = glwin;

	cursorEmpty = new BCursor(B_CURSOR_ID_NO_CURSOR);
	cursorStd = new BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);

	fView = new HaikuPlatformView(Bounds(), window->haiku.width, window->haiku.height, window);
	AddChild(fView);
}


HaikuPlatformWindow::~HaikuPlatformWindow()
{
	delete cursorEmpty;
	delete cursorStd;
}

void 
HaikuPlatformWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case 'mSHO':
			fView->SetViewCursor(cursorStd);
			break;
		case 'mHID':
			fView->SetViewCursor(cursorEmpty);
			break;
		case 'mNOR':
			fView->SetMouseMode(MOUSE_MODE_NORMAL);
			break;
		case 'mREL':
			fView->SetMouseMode(MOUSE_MODE_RELATIVE);
			break;
		case B_UNMAPPED_KEY_UP:
		case B_KEY_UP:
		case B_UNMAPPED_KEY_DOWN:
		case B_KEY_DOWN:
		case B_MOUSE_WHEEL_CHANGED:
		{
			BMessage *msg = new BMessage(*message);
			msg->AddPointer("window", (void*)window);
			PLATFORM_QUEUE->AddMessage(msg);
			break;
		}
		default:
			break;
	}
	BWindow::MessageReceived(message);
}

void
HaikuPlatformWindow::FrameResized(float width, float height)
{
	BMessage *msg = new BMessage(B_WINDOW_RESIZED);
	msg->AddPointer("window", (void*)window);
	msg->AddFloat("width", width);
	msg->AddFloat("height", height);
	PLATFORM_QUEUE->AddMessage(msg);
}

void
HaikuPlatformWindow::FrameMoved(BPoint point)
{
	BMessage *msg = new BMessage(B_WINDOW_MOVED);
	msg->AddPointer("window", (void*)window);
	msg->AddPoint("where", point);
	PLATFORM_QUEUE->AddMessage(msg);
}

void
HaikuPlatformWindow::WindowActivated(bool active)
{
	BMessage *msg = new BMessage(B_WINDOW_ACTIVATED);
	msg->AddPointer("window", (void*)window);
	msg->AddBool("active", active);
	PLATFORM_QUEUE->AddMessage(msg);
}

bool
HaikuPlatformWindow::QuitRequested()
{
	_glfwInputWindowCloseRequest(window);
	return false;
}
