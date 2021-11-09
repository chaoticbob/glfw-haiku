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

#include <GL/gl.h>

#include <Application.h>
#include <Window.h>
#include <Rect.h>
#include <View.h>
#include <Bitmap.h>
#include <Screen.h>
#include <Cursor.h>
#include <MessageQueue.h>
#include <InterfaceDefs.h>
#include <game/WindowScreen.h>

#include "haiku_platform_window.h"
#include "haiku_platform_view.h"

BView* _glfwGetHaikuView(_GLFWwindow* handle);

static int createNativeWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig)
{
	window->haiku.width = wndconfig->width;
	window->haiku.height = wndconfig->height;

	BRect winPos = BRect(100, 100, 100 + (wndconfig->width - 1), 100 + (wndconfig->height - 1));

	if (window->monitor) {
		BScreen scr(B_MAIN_SCREEN_ID);
    	window->haiku.width = scr.Frame().Width() + 1;
    	window->haiku.height = scr.Frame().Height() + 1;
    	winPos = BRect(0, 0, window->haiku.width, window->haiku.height);
	}

	uint32 flags = B_NOT_RESIZABLE | B_NOT_ZOOMABLE;

#if 0
	if (wndconfig->resizable && !window->monitor) {
		flags &= ~B_NOT_RESIZABLE;
		flags &= ~B_NOT_ZOOMABLE;
	}
#endif

	HaikuPlatformWindow *haiku_window = new HaikuPlatformWindow(winPos,
		wndconfig->title, B_TITLED_WINDOW, flags, window);

	window->haiku.object = (void*)(haiku_window);
	window->haiku.mouseGrab = GLFW_FALSE;
	
	if (window->monitor)
		haiku_window->Show();

    return GLFW_TRUE;
}

// Translates Haiku key modifiers into GLFW ones
//
static int translateFlags(int32 modificators)
{
    int mods = 0;

    if (modificators & B_SHIFT_KEY)
        mods |= GLFW_MOD_SHIFT;
    if (modificators & B_CONTROL_KEY)
        mods |= GLFW_MOD_CONTROL;
    if (modificators & B_COMMAND_KEY)
        mods |= GLFW_MOD_ALT;
    if (modificators & B_OPTION_KEY)
        mods |= GLFW_MOD_SUPER;
    if (modificators & B_CAPS_LOCK)
        mods |= GLFW_MOD_CAPS_LOCK;

    return mods;
}

// Translates a Haiku keycode to a GLFW keycode
//
static int translateKey(unsigned int key)
{
    if (key >= sizeof(_glfw.haiku.keycodes) / sizeof(_glfw.haiku.keycodes[0]))
        return GLFW_KEY_UNKNOWN;

    return _glfw.haiku.keycodes[key];
}

// Decode a Unicode code point from a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
static unsigned int decodeUTF8(const char** s)
{
    unsigned int ch = 0, count = 0;
    static const unsigned int offsets[] =
    {
        0x00000000u, 0x00003080u, 0x000e2080u,
        0x03c82080u, 0xfa082080u, 0x82082080u
    };

    do
    {
        ch = (ch << 6) + (unsigned char) **s;
        (*s)++;
        count++;
    } while ((**s & 0xc0) == 0x80);

    return ch - offsets[count - 1];
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_GLFWwindow* window)
{
    if (window->cursorMode == GLFW_CURSOR_NORMAL ||
	    	!PLATFORM_WINDOW(window)->IsActive()) {
    	PLATFORM_WINDOW(window)->PostMessage('mSHO');
    } else {
    	PLATFORM_WINDOW(window)->PostMessage('mHID');
    }
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_GLFWwindow* window)
{
    _glfw.haiku.disabledCursorWindow = window;
    _glfwPlatformGetCursorPos(window, &_glfw.haiku.restoreCursorPosX, &_glfw.haiku.restoreCursorPosY);
    updateCursorImage(window);
    _glfwCenterCursorInContentArea(window);
   	PLATFORM_WINDOW(window)->PostMessage('mREL');
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_GLFWwindow* window)
{
    _glfw.haiku.disabledCursorWindow = NULL;
    _glfwPlatformSetCursorPos(window, _glfw.haiku.restoreCursorPosX, _glfw.haiku.restoreCursorPosY);
    window->haiku.mouseGrab = GLFW_FALSE;
    updateCursorImage(window);
	PLATFORM_WINDOW(window)->PostMessage('mNOR');
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_OSMESA_CONTEXT_API || ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
        {
            if (!_glfwInitOSMesa())
                return GLFW_FALSE;
            if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else
        {
            _glfwInputError(GLFW_API_UNAVAILABLE, "Haiku: EGL not available");
            return GLFW_FALSE;
        }
    }

    return GLFW_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
	PLATFORM_WINDOW(window)->Lock();
	PLATFORM_WINDOW(window)->Quit();

    if (window->context.destroy)
        window->context.destroy(window);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
	PLATFORM_WINDOW(window)->SetTitle(title);
}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window, int count,
                                const GLFWimage* images)
{
}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
	_glfwInputWindowMonitor(window, monitor);
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
	BRect rect = PLATFORM_WINDOW(window)->Frame();
	if (xpos)
		*xpos = rect.left;
	if (ypos)
		*ypos = rect.top;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
	PLATFORM_WINDOW(window)->MoveTo(xpos, ypos);
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->haiku.width;
    if (height)
        *height = window->haiku.height;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    window->haiku.width = width;
    window->haiku.height = height;
    PLATFORM_WINDOW(window)->ResizeTo(width, height);
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int n, int d)
{
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = PLATFORM_VIEW(window)->Width();
    if (height)
        *height = PLATFORM_VIEW(window)->Height();	
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
	BRect rect = PLATFORM_WINDOW(window)->Frame();
	if (left)
		*left = rect.left;
	if (top)
		*top = rect.top;
	if (right)
		*right = rect.right;
	if (bottom)
		*bottom = rect.bottom;
}

void _glfwPlatformGetWindowContentScale(_GLFWwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
	if (!PLATFORM_WINDOW(window)->IsMinimized())
		PLATFORM_WINDOW(window)->Minimize(true);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
	if (PLATFORM_WINDOW(window)->IsMinimized())
		PLATFORM_WINDOW(window)->Minimize(false);
}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window)
{
}

int _glfwPlatformWindowMaximized(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowHovered(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformFramebufferTransparent(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwPlatformSetWindowResizable(_GLFWwindow* window, GLFWbool enabled)
{
#if 0
	uint32 flags = ((BWindow*)(window->haiku.wnd))->Flags();
	if (enabled) {
		flags &= ~B_NOT_RESIZABLE;
		flags &= ~B_NOT_ZOOMABLE;
	} else {
		flags |= B_NOT_RESIZABLE | B_NOT_ZOOMABLE;
	}
	((BWindow*)(window->haiku.wnd))->SetFlags(flags);
#endif
}

void _glfwPlatformSetWindowDecorated(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformSetWindowFloating(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwPlatformGetWindowOpacity(_GLFWwindow* window)
{
    return 1.f;
}

void _glfwPlatformSetWindowOpacity(_GLFWwindow* window, float opacity)
{
}

void _glfwPlatformSetRawMouseMotion(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwPlatformRawMouseMotionSupported(void)
{
    return GLFW_FALSE;
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
	if (PLATFORM_WINDOW(window)->IsHidden())
		PLATFORM_WINDOW(window)->Show();
}


void _glfwPlatformRequestWindowAttention(_GLFWwindow* window)
{
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
	if (PLATFORM_WINDOW(window)->IsHidden())
		PLATFORM_WINDOW(window)->Show();
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
	if (!PLATFORM_WINDOW(window)->IsHidden())
		PLATFORM_WINDOW(window)->Hide();
}

void _glfwPlatformFocusWindow(_GLFWwindow* window)
{
	if (!PLATFORM_WINDOW(window)->IsActive())
		PLATFORM_WINDOW(window)->Activate(true);
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    return PLATFORM_WINDOW(window)->IsActive() ? GLFW_TRUE : GLFW_FALSE;
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    return PLATFORM_WINDOW(window)->IsHidden() ? GLFW_FALSE : GLFW_TRUE;
}

void _glfwPlatformPollEvents(void)
{
	PLATFORM_QUEUE->Lock();
	while(!PLATFORM_QUEUE->IsEmpty()) {
		BMessage *message = PLATFORM_QUEUE->NextMessage();
		_GLFWwindow* window = (_GLFWwindow*)(message->GetPointer("window"));
		switch(message->what) {
			case B_MOUSE_DOWN:
			{
				int32 mod = modifiers();
				uint32 buttons = message->FindInt32("buttons");

				if (buttons & B_PRIMARY_MOUSE_BUTTON) {
					if (window->cursorMode == GLFW_CURSOR_DISABLED && window->haiku.mouseGrab == GLFW_FALSE) {
						window->haiku.mouseGrab = GLFW_TRUE;
						disableCursor(window);
					}
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, translateFlags(mod));
				}
			
				if (buttons & B_SECONDARY_MOUSE_BUTTON)
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, translateFlags(mod));
			
				if (buttons & B_TERTIARY_MOUSE_BUTTON)
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, translateFlags(mod));

				window->haiku.lastMouseButtons = buttons;

				break;
			}
			case B_MOUSE_UP:
			{
				int32 mod = modifiers();
				uint32 buttons = message->FindInt32("buttons");

				if ((window->haiku.lastMouseButtons & B_PRIMARY_MOUSE_BUTTON) && !(buttons & B_PRIMARY_MOUSE_BUTTON))
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, translateFlags(mod));
			
				if ((window->haiku.lastMouseButtons & B_SECONDARY_MOUSE_BUTTON) && !(buttons & B_SECONDARY_MOUSE_BUTTON))
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, translateFlags(mod));
			
				if ((window->haiku.lastMouseButtons & B_TERTIARY_MOUSE_BUTTON) && !(buttons & B_TERTIARY_MOUSE_BUTTON))
					_glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, translateFlags(mod));

				break;
			}
			case B_MOUSE_MOVED:
			{
				uint32 transit;
				message->FindInt32("be:transit", (int32*)&transit);
				BPoint where;
				message->FindPoint("be:view_where", &where);

				if (transit == B_ENTERED_VIEW) {
					_glfwInputCursorEnter(window, GLFW_TRUE);
				} else if (transit == B_EXITED_VIEW) {
					_glfwInputCursorEnter(window, GLFW_FALSE);
				} else if (transit == B_INSIDE_VIEW || transit == B_OUTSIDE_VIEW) {
					if (window->cursorMode == GLFW_CURSOR_DISABLED) {
						if (PLATFORM_WINDOW(window)->IsActive() && window->haiku.mouseGrab == GLFW_TRUE) {
							int cx = PLATFORM_WINDOW(window)->Bounds().Width() / 2;
							int cy = PLATFORM_WINDOW(window)->Bounds().Height() / 2;
							float dx = where.x - cx;
							float dy = where.y - cy;
							BPoint center((PLATFORM_WINDOW(window)->Frame().right + PLATFORM_WINDOW(window)->Frame().left) / 2,
								(PLATFORM_WINDOW(window)->Frame().bottom + PLATFORM_WINDOW(window)->Frame().top) / 2);
							set_mouse_position(center.x, center.y);
							_glfwInputCursorPos(window, window->virtualCursorPosX + dx, window->virtualCursorPosY + dy);
						}
					} else
						_glfwInputCursorPos(window, where.x, where.y);
				}			
				break;
			}
			case B_MOUSE_WHEEL_CHANGED:
			{
				float shift_x=0;
				float shift_y=0;
				if (message->FindFloat("be:wheel_delta_x", &shift_x) != B_OK)
					shift_x = 0;
				if (message->FindFloat("be:wheel_delta_y", &shift_y) != B_OK)
					shift_y = 0;
				if (window)
					_glfwInputScroll(window, shift_x, -shift_y);
				break;
			}
			case B_WINDOW_ACTIVATED:
			{
				bool active = message->FindBool("active");
				if (active) {
					_glfwInputWindowFocus(window, GLFW_TRUE);
					if (window->cursorMode == GLFW_CURSOR_DISABLED && window->haiku.mouseGrab == GLFW_TRUE)
						disableCursor(window);
				} else {
					if (window->cursorMode == GLFW_CURSOR_DISABLED)
						enableCursor(window);

					if (window->monitor && window->autoIconify)
						_glfwPlatformIconifyWindow(window);

					_glfwInputWindowFocus(window, GLFW_FALSE);
				}
				break;
			}
			case B_WINDOW_MOVED:
			{
				BPoint point = message->FindPoint("where");
				_glfwInputWindowPos(window, point.x, point.y);
				break;
			}
			case B_WINDOW_RESIZED:
			{
				float width = message->FindFloat("width");
				float height = message->FindFloat("height");
				if (!window->monitor)
					_glfwInputFramebufferSize(window, width, height);

				_glfwInputWindowSize(window, width, height);
				window->haiku.width = width;
				window->haiku.height = height;
				break;
			}
			case B_UNMAPPED_KEY_UP:
			case B_KEY_UP:
			case B_UNMAPPED_KEY_DOWN:
			case B_KEY_DOWN:
			{
				const int action = (message->what == B_UNMAPPED_KEY_UP || message->what == B_KEY_UP) ? GLFW_RELEASE : GLFW_PRESS;
				const int mods = translateFlags(modifiers());
				
				int32 scancode;
				message->FindInt32("key", &scancode);	
				int32 key = translateKey(scancode);

				_glfwInputKey(window, key, key, action, mods);

				if (action == GLFW_PRESS) {
					const char* bytes;
					if(message->FindString("bytes", &bytes) == B_OK) {
						const int plain = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));
						_glfwInputChar(window, decodeUTF8(&bytes), mods, plain);
					}
				}
				break;
			}
		}
	}
	PLATFORM_QUEUE->Unlock();
}

void _glfwPlatformWaitEvents(void)
{
	while(PLATFORM_QUEUE->IsEmpty())
		snooze(10);

	_glfwPlatformPollEvents();
}

void _glfwPlatformWaitEventsTimeout(double timeout)
{
	bigtime_t enterTime = system_time();

	while(PLATFORM_QUEUE->IsEmpty()) {
		bigtime_t elapsed = system_time() - enterTime;
		if (elapsed > timeout * 1000000)
			break;
		snooze(10);
	}

	_glfwPlatformPollEvents();
}

void _glfwPlatformPostEmptyEvent(void)
{
	BMessage *msg = new BMessage(B_PULSE);
	PLATFORM_QUEUE->AddMessage(msg);
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
	uint32 buttons;
	BPoint screenWhere;
	get_mouse(&screenWhere, &buttons);
	BPoint mousePos = screenWhere - PLATFORM_WINDOW(window)->Frame().LeftTop();

	if (xpos)
		*xpos = mousePos.x;
	if (ypos)
		*ypos = mousePos.y;
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
	set_mouse_position(PLATFORM_WINDOW(window)->Frame().left + x, PLATFORM_WINDOW(window)->Frame().top + y);
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
	if (mode == GLFW_CURSOR_DISABLED) {
		if (_glfwPlatformWindowFocused(window)) {
			disableCursor(window);
			window->haiku.mouseGrab = GLFW_TRUE;
		}
	} else if (_glfw.haiku.disabledCursorWindow == window)
		enableCursor(window);

	updateCursorImage(window);
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    return GLFW_TRUE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwPlatformSetClipboardString(const char* string)
{
}

const char* _glfwPlatformGetClipboardString(void)
{
    return NULL;
}

const char* _glfwPlatformGetScancodeName(int scancode)
{
    return "";
}

int _glfwPlatformGetKeyScancode(int key)
{
	return _glfw.haiku.scancodes[key];
}

void _glfwPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.EXT_haiku_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_EXT_haiku_surface";
}

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
   	HaikuPlatformView *view = PLATFORM_VIEW(window);
   	glReadPixels(0, 0, view->Width(), view->Height(), GL_BGRA, GL_UNSIGNED_BYTE, view->GetBuffer());
   	view->Repaint();   	
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_TRUE;
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    VkResult err;
    VkHaikuSurfaceCreateInfoEXT sci;
    PFN_vkCreateHaikuSurfaceEXT vkCreateHaikuSurfaceEXT;

    vkCreateHaikuSurfaceEXT = (PFN_vkCreateHaikuSurfaceEXT)
        vkGetInstanceProcAddr(instance, "vkCreateHaikuSurfaceEXT");
    if (!vkCreateHaikuSurfaceEXT)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Haiku: Vulkan instance missing VK_EXT_haiku_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = (VkStructureType)0; 
    sci.pView = _glfwGetHaikuView(window);

    err = vkCreateHaikuSurfaceEXT(instance, &sci, allocator, surface);
    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

BView* _glfwGetHaikuView(_GLFWwindow* window)
{
    HaikuPlatformView *view = PLATFORM_VIEW(window);
    return view;
}

extern "C" {

GLFWAPI BView* glfwGetHaikuView(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*)handle;
    return _glfwGetHaikuView(window);
}

}
