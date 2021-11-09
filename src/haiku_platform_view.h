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

#ifndef _HAIKU_PLATFORM_VIEW_H
#define _HAIKU_PLATFORM_VIEW_H

extern "C" {
#include "internal.h"
}

#include <SupportDefs.h>
#include <Window.h>
#include <Bitmap.h>
#include <View.h>
#include <Cursor.h>
#include <Rect.h>
#include <kernel/OS.h>
#include <MessageQueue.h>
#include <game/WindowScreen.h>

#define MOUSE_MODE_NORMAL	0
#define MOUSE_MODE_RELATIVE	1

class HaikuPlatformView : public BView 
{
 public:
		HaikuPlatformView(BRect rect, int width, int height, _GLFWwindow* win);
		~HaikuPlatformView();

//		virtual void		MouseDown(BPoint point);
//		virtual void		MouseUp(BPoint point);
//		virtual void 		MouseMoved(BPoint where, uint32 code, const BMessage *message);
		virtual void		MessageReceived(BMessage *message);
		virtual void		Draw(BRect r);

		void 				Repaint();

		void				SetMouseMode(int32 mode);

		char*				GetBuffer() { return pixelBuffer; }
		uint32				GetBufferSize() { return bufferBitmap->BitsLength(); }

		void				ResizeBitmap(int width, int height);

		int					Width() { return pixelBufferWidth; }
		int					Height() { return pixelBufferHeight; }

 private:
		_GLFWwindow*		window;

		BBitmap*			bufferBitmap;

 		int					pixelBufferWidth;
 		int					pixelBufferHeight;
		char*				pixelBuffer;

//		BPoint				lastMousePosition;
//		uint32 				lastButtons;
		int32				mouseMode;
};

#endif //_HAIKU_PLATFORM_VIEW_H
