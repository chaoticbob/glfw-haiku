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

#include <stdio.h>

#include "haiku_platform_view.h"

HaikuPlatformView::HaikuPlatformView(BRect rect, int width, int height, _GLFWwindow* win) :
	BView(rect, "HaikuPlatformView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	mouseMode = MOUSE_MODE_NORMAL;
	pixelBufferWidth = width;
	pixelBufferHeight = height;
	window = win;

	BRect	fbRect = BRect(0, 0, pixelBufferWidth - 1, pixelBufferHeight - 1);
	bufferBitmap = new BBitmap(fbRect, B_RGB32, true);
	pixelBuffer = (char*)malloc(bufferBitmap->BitsLength());
}

HaikuPlatformView::~HaikuPlatformView()
{
	if (bufferBitmap)
		delete bufferBitmap;
	if (pixelBuffer)
		free(pixelBuffer);
}

void 
HaikuPlatformView::MessageReceived(BMessage *message)
{
	BMessageQueue *queue = (BMessageQueue*)(_glfw.haiku.messageQueue);

	switch (message->what) {
		case B_MOUSE_DOWN:
		{
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
			BMessage *msg = new BMessage(*message);
			msg->AddPointer("window", (void*)window);
			queue->AddMessage(msg);
			return;
		}
		case B_MOUSE_UP:
		{
			BPoint where;
			uint32 buttons;
			GetMouse(&where, &buttons);
			BMessage *msg = new BMessage(*message);
			msg->AddPointer("window", (void*)window);
			msg->AddInt32("buttons", buttons);
			queue->AddMessage(msg);
			return;
		}
		case B_MOUSE_MOVED:
		{
			BMessage *msg = new BMessage(*message);
			msg->AddPointer("window", (void*)window);
			queue->AddMessage(msg);
			return;
		}
		default:
			break;
	}
	BView::MessageReceived(message);
}

void 
HaikuPlatformView::Draw(BRect rect)
{
	SetDrawingMode(B_OP_COPY);
	DrawBitmap(bufferBitmap, rect, rect);
}

void
HaikuPlatformView::SetMouseMode(int32 mode)
{
	mouseMode = mode;

	if (mouseMode == MOUSE_MODE_RELATIVE)
		SetEventMask(B_POINTER_EVENTS | B_KEYBOARD_EVENTS, B_NO_POINTER_HISTORY);
	else
		SetEventMask(0, 0);
}

void 
HaikuPlatformView::Repaint()
{
	uint32 bytesPerRow = bufferBitmap->BytesPerRow();

   	char *s_ptr = pixelBuffer + ((pixelBufferHeight - 1) * pixelBufferWidth) * 4;
   	char *d_ptr = (char*)bufferBitmap->Bits();

   	for (int i=0; i < pixelBufferHeight; i++, d_ptr += bytesPerRow, s_ptr -= bytesPerRow)
		memcpy(d_ptr, s_ptr, bytesPerRow);

	if (LockLooperWithTimeout(10000) == B_OK) {
		SetDrawingMode(B_OP_COPY);
		DrawBitmap(bufferBitmap);
		UnlockLooper();
	}
}

void
HaikuPlatformView::ResizeBitmap(int width, int height)
{
	if(width == pixelBufferWidth && height == pixelBufferHeight)
		return;

	if(LockLooper()) {
	 	delete bufferBitmap;
	 	free(pixelBuffer);

		pixelBufferWidth = width;
		pixelBufferHeight = height;

		BRect	fbRect = BRect(0, 0, pixelBufferWidth - 1, pixelBufferHeight - 1);
		bufferBitmap = new BBitmap(fbRect, B_RGB32, true);
		pixelBuffer = (char*)malloc(bufferBitmap->BitsLength());

	 	UnlockLooper();
	} 	 
}
