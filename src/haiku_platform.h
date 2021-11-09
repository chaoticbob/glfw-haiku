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

#include <dlfcn.h>

typedef VkFlags VkHaikuSurfaceCreateFlagsEXT;

typedef struct VkHaikuSurfaceCreateInfoEXT {
    VkStructureType                     sType;
    const void*                         pNext;
    VkHaikuSurfaceCreateFlagsEXT        flags;
    void*                               pView;
} VkHaikuSurfaceCreateInfoEXT;

typedef VkResult (*PFN_vkCreateHaikuSurfaceEXT)(VkInstance instance, const VkHaikuSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

#define _GLFW_PLATFORM_WINDOW_STATE _GLFWwindowHaiku haiku

#define _GLFW_PLATFORM_WINDOW_STATE _GLFWwindowHaiku haiku

#define _GLFW_PLATFORM_CONTEXT_STATE         struct { int dummyContext; }
#define _GLFW_PLATFORM_MONITOR_STATE         struct { int dummyMonitor; }
#define _GLFW_PLATFORM_CURSOR_STATE          struct { int dummyCursor; }
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE struct { int dummyLibraryContext; }
#define _GLFW_EGL_CONTEXT_STATE              struct { int dummyEGLContext; }
#define _GLFW_EGL_LIBRARY_CONTEXT_STATE      struct { int dummyEGLLibraryContext; }

#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE  _GLFWlibraryHaiku haiku

#include "osmesa_context.h"
#include "posix_time.h"
#include "posix_thread.h"
#include "haiku_joystick.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

// Haiku-specific per-window data
//
typedef struct _GLFWwindowHaiku
{
    int 		width;
    int 		height;
    void*		object;
    uint32_t	lastMouseButtons;
    int			mouseGrab;
} _GLFWwindowHaiku;


// Cocoa-specific global data
//
typedef struct _GLFWlibraryHaiku
{
	_GLFWwindow*	disabledCursorWindow;

	double			restoreCursorPosX, restoreCursorPosY;

	void*			messageQueue;

	char			keynames[GLFW_KEY_LAST + 1][17];
	short int		keycodes[256];
	short int		scancodes[GLFW_KEY_LAST + 1];
} _GLFWlibraryHaiku;
