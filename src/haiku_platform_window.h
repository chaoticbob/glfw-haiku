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

#ifndef _HAIKU_PLATFORM_WINDOW_H
#define _HAIKU_PLATFORM_WINDOW_H

#include <stdio.h>

#include <OS.h>
#include <View.h>
#include <Window.h>
#include <SupportDefs.h>
#include <MessageQueue.h>

#include "haiku_platform_view.h"

class HaikuPlatformWindow : public BWindow {
	public:
						HaikuPlatformWindow(BRect frame, const char* title, window_type type, uint32 flags, _GLFWwindow* glwin);
		virtual			~HaikuPlatformWindow();

		virtual void 	MessageReceived(BMessage *message);
		virtual	bool	QuitRequested();
		virtual void	FrameResized(float width, float height);
		virtual void 	FrameMoved(BPoint point);
		virtual void	WindowActivated(bool active);

		HaikuPlatformView*	View(void) { return fView; }

	private:
		_GLFWwindow* window;
		HaikuPlatformView *fView;
		BCursor *cursorEmpty;
		BCursor *cursorStd;
};

#define PLATFORM_WINDOW(window) ((HaikuPlatformWindow*)(window->haiku.object))
#define PLATFORM_VIEW(window) (PLATFORM_WINDOW(window)->View())
#define PLATFORM_QUEUE ((BMessageQueue*)(_glfw.haiku.messageQueue))

#endif //_HAIKU_PLATFORM_WINDOW_H


