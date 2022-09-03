
#include "winx.h"

#define SET_HINT(HINT_ENUM, HINT_VAR) case HINT_ENUM: HINT_VAR = value; break;
#define WINX_CONTEXT_ASSERT(function) if(!winx) { winxErrorMsg = (char*) (function ": No active winx context!"); return; }


// dummy functions
void WinxDummyMouseEventHandle(int x, int y) {}
void WinxDummyButtonEventHandle(int type, int button) {}
void WinxDummyKeybordEventHandle(int type, int key) {}
void WinxDummyScrollEventHandle(int scroll) {}
void WinxDummyCloseEventHandle() {}
void WinxDummyResizeEventHandle(int width, int height) {}

// hints
static int __winx_hint_vsync = 0;
static int __winx_hint_red_bits = 8;
static int __winx_hint_green_bits = 8;
static int __winx_hint_blue_bits = 8;
static int __winx_hint_alpha_bits = 8;
static int __winx_hint_depth_bits = 24;
static int __winx_hint_stencil_bits = 1;
static int __winx_hint_opengl_major = 3;
static int __winx_hint_opengl_minor = 0;
static int __winx_hint_opengl_core = 1;
static int __winx_hint_opengl_debug = 0;
static int __winx_hint_opengl_robust = 0;
static int __winx_hint_multisamples = 0;

// current error message
static char* winxErrorMsg = NULL;

char* winxGetError() {
	char* copy = winxErrorMsg;
	winxErrorMsg = NULL;

	return copy;
}

void winxHint(int hint, int value) {
	switch (hint) {
		SET_HINT(WINX_HINT_VSYNC, __winx_hint_vsync);
		SET_HINT(WINX_HINT_RED_BITS, __winx_hint_red_bits);
		SET_HINT(WINX_HINT_GREEN_BITS,	__winx_hint_green_bits);
		SET_HINT(WINX_HINT_BLUE_BITS, __winx_hint_blue_bits);
		SET_HINT(WINX_HINT_ALPHA_BITS, __winx_hint_alpha_bits);
		SET_HINT(WINX_HINT_DEPTH_BITS, __winx_hint_depth_bits);
		SET_HINT(WINX_HINT_STENCIL_BITS, __winx_hint_stencil_bits);
		SET_HINT(WINX_HINT_OPENGL_MAJOR, __winx_hint_opengl_major);
		SET_HINT(WINX_HINT_OPENGL_MINOR, __winx_hint_opengl_minor);
		SET_HINT(WINX_HINT_OPENGL_CORE, __winx_hint_opengl_core);
		SET_HINT(WINX_HINT_OPENGL_DEBUG, __winx_hint_opengl_debug);
		SET_HINT(WINX_HINT_OPENGL_ROBUST, __winx_hint_opengl_robust);
		SET_HINT(WINX_HINT_MULTISAMPLES, __winx_hint_multisamples);
	}
}

#undef SET_HINT


// begin winx GLX implementation
#if defined(WINX_GLX)

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

// copied from glxext.h
typedef int ( *PFNGLXSWAPINTERVALMESAPROC) (unsigned int interval);
typedef void ( *PFNGLXSWAPINTERVALEXTPROC) (Display *dpy, GLXDrawable drawable, int interval);
typedef GLXContext ( *PFNGLXCREATECONTEXTATTRIBSARBPROC) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);

static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

typedef struct {
	Display* display;
	Window window;
	GLXContext context;
	Atom wm_delete_window;
	Atom net_wm_icon;
	Atom cardinal;

	WinxMouseEventHandle mouse;
	WinxButtonEventHandle button;
	WinxKeybordEventHandle keyboard;
	WinxScrollEventHandle scroll;
	WinxCloseEventHandle close;
	WinxResizeEventHandle resize;
} WinxHandle;

static WinxHandle* winx = NULL;

static __GLXextFuncPtr winxGetProc(const char* name) {
	if (winxErrorMsg == NULL) {
		__GLXextFuncPtr proc = glXGetProcAddress((const unsigned char*) name);

		if (proc == NULL) {
			winxErrorMsg = (char*) "glXGetProcAddress: Failed to load function!";
		}

		return proc;
	}

	return NULL;
}

bool winxOpen(int width, int height, const char* title) {
	winx = (WinxHandle*) malloc(sizeof(WinxHandle));

	// set dummy function pointers
	winx->button = WinxDummyButtonEventHandle;
	winx->mouse = WinxDummyMouseEventHandle;
	winx->keyboard = WinxDummyKeybordEventHandle;
	winx->scroll = WinxDummyScrollEventHandle;
	winx->close = WinxDummyCloseEventHandle;
	winx->resize = WinxDummyResizeEventHandle;

	// get display handle
	winx->display = XOpenDisplay(NULL);
	if (!winx->display) {
		winxErrorMsg = (char*) "XOpenDisplay: Failed to acquire display handle!";
		return false;
	}

	// GLX attributes
	int attributes[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, true,
		GLX_RED_SIZE, __winx_hint_red_bits,
		GLX_GREEN_SIZE, __winx_hint_green_bits,
		GLX_BLUE_SIZE, __winx_hint_blue_bits,
		GLX_DEPTH_SIZE, __winx_hint_depth_bits,
		GLX_STENCIL_SIZE, __winx_hint_stencil_bits,
		GLX_SAMPLE_BUFFERS, __winx_hint_multisamples ? 1 : 0,
		GLX_SAMPLES, __winx_hint_multisamples,
		None
	};

	int screen = DefaultScreen(winx->display);
	Window root = RootWindow(winx->display, screen);

	// find frame buffer config matching our attributes
	int count;
	GLXFBConfig *fbconfigs = glXChooseFBConfig(winx->display, screen, attributes, &count);
	if (!fbconfigs || !count) {
		winxErrorMsg = (char*) "glXChooseFBConfig: Failed to choose a frame buffer config!";
		return false;
	}

	// find visual based on framebuffer's config
	XVisualInfo* info = glXGetVisualFromFBConfig(winx->display, fbconfigs[0]);
	if (!info) {
		winxErrorMsg = (char*) "glXGetVisualFromFBConfig: Failed to choose a visual!";
		return false;
	}

	// set X11 window attributes
	XSetWindowAttributes x11_attributes;
	x11_attributes.background_pixel = 0;
	x11_attributes.border_pixel = 0;
	x11_attributes.colormap = XCreateColormap(winx->display, root, info->visual, AllocNone);
	x11_attributes.event_mask = StructureNotifyMask | ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;

	unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	// finally create X11 window
	winx->window = XCreateWindow(winx->display, root, 0, 0, width, height, 0, info->depth, InputOutput, info->visual, mask, &x11_attributes);

	// set name
	winxSetTitle(title);

	// show X11 window
	XMapWindow(winx->display, winx->window);

	// create GLX context
	GLXContext context = glXCreateContext(winx->display, info, NULL, 1);
	if (!context) {
		winxErrorMsg = (char*) "glXCreateContext: Failed to create GLX context!";
		return false;
	}

	glXMakeCurrent(winx->display, winx->window, context);

	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC) winxGetProc("glXCreateContextAttribsARB");
	glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddress((const unsigned char*) "glXSwapIntervalEXT"); // optional
	glXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC) glXGetProcAddress((const unsigned char*) "glXSwapIntervalMESA"); // optional

	if (winxErrorMsg != NULL) {
		return false;
	}

	glXMakeCurrent(winx->display, 0, 0);
	glXDestroyContext(winx->display, context);

	XFree(info);

	int flags = 0;

	if (__winx_hint_opengl_debug) flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
	if (__winx_hint_opengl_robust) flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;

	int context_attributes[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, __winx_hint_opengl_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, __winx_hint_opengl_minor,
		GLX_CONTEXT_PROFILE_MASK_ARB, __winx_hint_opengl_core ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, flags,
		None
	};

	winx->context = glXCreateContextAttribsARB(winx->display, fbconfigs[0], NULL, true, context_attributes);
	if (!winx->context) {
		winxErrorMsg = (char*) "glXCreateContextAttribsARB: Failed to create context";
		return false;
	}

	glXMakeCurrent(winx->display, winx->window, winx->context);

	// needed for window icon
	winx->net_wm_icon = XInternAtom(winx->display, "_NET_WM_ICON", false);
    winx->cardinal = XInternAtom(winx->display, "CARDINAL", false);

	// needed to handle the close button
	winx->wm_delete_window = XInternAtom(winx->display, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(winx->display, winx->window, &(winx->wm_delete_window), 1);

	// set vsync
	winxSetVsync(__winx_hint_vsync);

	return true;
}

void winxPollEvents() {
	while( XPending(winx->display) > 0 ) {

		XEvent event;
		XNextEvent(winx->display, &event);

		switch (event.type) {

			case ClientMessage:
				if (event.xclient.data.l[0] == winx->wm_delete_window) {
					winx->close();
					break;
				}

			case KeyPress:
				winx->keyboard(WINX_PRESSED, XLookupKeysym(&event.xkey, 0));
				break;

			case KeyRelease:
				winx->keyboard(WINX_RELEASED, XLookupKeysym(&event.xkey, 0));
				break;

			case ButtonPress:
				if (event.xbutton.button == Button4) {
					winx->scroll(1);
					break;
				}

				if (event.xbutton.button == Button5) {
					winx->scroll(-1);
					break;
				}

				winx->button(WINX_PRESSED, event.xbutton.button);
				break;

			case ButtonRelease:
				winx->button(WINX_RELEASED, event.xbutton.button);
				break;

			case MotionNotify:
				winx->mouse(event.xmotion.x_root, event.xmotion.y_root);
				break;

			case ConfigureNotify:
				winx->resize(event.xconfigure.width, event.xconfigure.height);
				break;

			default:
				break;

		}

	}
}

void winxSwapBuffers() {
	glXSwapBuffers(winx->display, winx->window);
}

void winxClose() {
	glXDestroyContext(winx->display, winx->context);
	XDestroyWindow(winx->display, winx->window);
	XCloseDisplay(winx->display);

	free(winx);
	winx = NULL;
}

void winxSetTitle(const char* title) {
	WINX_CONTEXT_ASSERT("winxSetTitle");

	XStoreName(winx->display, winx->window, title);
	XSetIconName(winx->display, winx->window, title);
}

void winxSetIcon(int width, int height, unsigned char* buffer) {
	WINX_CONTEXT_ASSERT("winxSetIcon");

	// We need to convert RGBA byte array to a suported format
	// X11 expects the icon in format [[long: width] [long: height] [long: bgra]...]...

	const int size = width * height;
	unsigned long* icon = (unsigned long*) malloc(sizeof(long) * (size + 2));

	icon[0] = width;
	icon[1] = height;

	for (int i = 0, j = 2; i < size * 4; i += 4) {
		icon[j ++] = buffer[i + 2] | buffer[i + 1] << 8 | buffer[i + 0] << 16 | buffer[i + 3] << 24;
	}

	XChangeProperty(winx->display, winx->window, winx->net_wm_icon, winx->cardinal, 32, PropModeReplace, (const unsigned char*) icon, size + 2);

	free(icon);
}

void winxSetVsync(int vsync) {
	WINX_CONTEXT_ASSERT("winxSetVsync");

	if (glXSwapIntervalEXT) {
		glXSwapIntervalEXT(winx->display, glXGetCurrentDrawable(), __winx_hint_vsync);
	} else {
		if (glXSwapIntervalMESA) {
			glXSwapIntervalMESA(__winx_hint_vsync == WINX_VSYNC_ADAPTIVE ? WINX_VSYNC_ENABLED : __winx_hint_vsync);
		}
	}
}

#endif // GLX

#if defined(WINX_WINAPI)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <gl/GL.h>

/* copied from wglext.h */
typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);

#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_ALPHA_BITS_ARB                0x201B
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_SWAP_EXCHANGE_ARB             0x2028
#define WGL_SWAP_METHOD_ARB               0x2007
#define WGL_SWAP_COPY_ARB                 0x2029
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_RED_BITS_ARB                  0x2015
#define WGL_GREEN_BITS_ARB                0x2017
#define WGL_BLUE_BITS_ARB                 0x2019
#define WGL_STENCIL_BITS_ARB              0x2023

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

typedef struct {
	HWND hndl;
	HDC device;
	HGLRC context;

	WinxMouseEventHandle mouse;
	WinxButtonEventHandle button;
	WinxKeybordEventHandle keyboard;
	WinxScrollEventHandle scroll;
	WinxCloseEventHandle close;
	WinxResizeEventHandle resize;
} WinxHandle;

static WinxHandle* winx = NULL;

static PROC winxGetProc(LPCSTR name) {
	if (winxErrorMsg == NULL) {
		PROC proc = wglGetProcAddress(name);

		if (proc == NULL) {
			winxErrorMsg = (char*) "wglGetProcAddress: Failed to load function!";
		}

		return proc;
	}

	return NULL;
}

static LRESULT CALLBACK winxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch (message) {
		case WM_KEYDOWN:
			winx->keyboard(WINX_PRESSED, wParam);
			break;

		case WM_KEYUP:
			winx->keyboard(WINX_RELEASED, wParam);
			break;

		case WM_MOUSEMOVE:
			winx->mouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;

		case WM_LBUTTONDOWN:
			winx->button(WINX_PRESSED, WXB_LEFT);
			break;

		case WM_LBUTTONUP:
			winx->button(WINX_RELEASED, WXB_LEFT);
			break;

		case WM_MBUTTONDOWN:
			winx->button(WINX_PRESSED, WXB_CENTER);
			break;

		case WM_MBUTTONUP:
			winx->button(WINX_RELEASED, WXB_CENTER);
			break;

		case WM_RBUTTONDOWN:
			winx->button(WINX_PRESSED, WXB_RIGHT);
			break;

		case WM_RBUTTONUP:
			winx->button(WINX_RELEASED, WXB_RIGHT);
			break;

		case WM_MOUSEWHEEL:
			winx->scroll(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			break;

		case WM_CLOSE:
			winx->close();
			break;

		case WM_SIZE:
			winx->resize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			result = DefWindowProcA(hWnd, message, wParam, lParam);
	}

	return result;
}

bool winxOpen(int width, int height, const char* title) {

	winx = (WinxHandle*) malloc(sizeof(WinxHandle));
	HINSTANCE hinstance = GetModuleHandle(NULL);

	// set dummy function pointers
	winx->button = WinxDummyButtonEventHandle;
	winx->mouse = WinxDummyMouseEventHandle;
	winx->keyboard = WinxDummyKeybordEventHandle;
	winx->scroll = WinxDummyScrollEventHandle;
	winx->close = WinxDummyCloseEventHandle;
	winx->resize = WinxDummyResizeEventHandle;

	// register window class
	const char* clazz = "WinxOpenGLClass";

	WNDCLASSEXA wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = (WNDPROC) winxWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinstance;
	wcex.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = clazz;
	wcex.hIconSm = NULL;

	if (!RegisterClassExA(&wcex)) {
		winxErrorMsg = (char*) "RegisterClassExA: Failed to register window class!";
		return false;
	}

	HWND fakeHndl;
	HDC fakeDeviceContext;
	HGLRC fakeRenderContext;

	// create temporary window to get WGL context
	fakeHndl = CreateWindowA(clazz, "WINX", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, hinstance, NULL);
	if (!fakeHndl) {
		winxErrorMsg = (char*) "CreateWindowA: Failed to create temporary window!";
		return false;
	}

	fakeDeviceContext = GetDC(fakeHndl);
	if (!fakeDeviceContext) {
		winxErrorMsg = (char*) "GetDC: Failed to create temporary device context!";
		return false;
	}

	PIXELFORMATDESCRIPTOR descriptor = {0};
	descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	descriptor.nVersion = 1;
	descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	descriptor.iPixelType = PFD_TYPE_RGBA;
	descriptor.cRedBits = __winx_hint_red_bits;
	descriptor.cGreenBits = __winx_hint_green_bits;
	descriptor.cBlueBits = __winx_hint_blue_bits;
	descriptor.cAlphaBits = __winx_hint_alpha_bits;
	descriptor.cDepthBits = __winx_hint_depth_bits;
	int fakePixelFormat = ChoosePixelFormat(fakeDeviceContext, &descriptor);

	if (!fakePixelFormat) {
		winxErrorMsg = (char*) "ChoosePixelFormat: Failed to choose a pixel format!";
		return false;
	}

	if (!SetPixelFormat(fakeDeviceContext, fakePixelFormat, &descriptor)) {
		winxErrorMsg = (char*) "SetPixelFormat: Failed to select a pixel format!";
		return false;
	}

	fakeRenderContext = wglCreateContext(fakeDeviceContext);
	if (!fakeRenderContext) {
		winxErrorMsg = (char*) "wglCreateContext: Failed to create temporary render context!";
		return false;
	}

	// open real window
	winx->hndl = CreateWindowA(clazz, title, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, hinstance, NULL);
	if (!winx->hndl) {
		winxErrorMsg = (char*) "CreateWindowA: Failed to create window!";
		return false;
	}

	// create context
	winx->device = GetDC(winx->hndl);
	if (!winx->device) {
		winxErrorMsg = (char*) "GetDC: Failed to create device context!";
		return false;
	}

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

	if (!wglMakeCurrent(fakeDeviceContext, fakeRenderContext)) {
		winxErrorMsg = (char*) "wglMakeCurrent: Failed to select temporary context!";
		return false;
	}

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) winxGetProc("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) winxGetProc("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); // optional

	if (winxErrorMsg != NULL) {
		// winxGetProc set the error message
		return false;
	}

	int pixelFormat;
	BOOL status;
	UINT numFormats;

	const int pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_RED_BITS_ARB, __winx_hint_red_bits,
		WGL_GREEN_BITS_ARB, __winx_hint_green_bits,
		WGL_BLUE_BITS_ARB, __winx_hint_blue_bits,
		WGL_ALPHA_BITS_ARB, __winx_hint_alpha_bits,
		WGL_DEPTH_BITS_ARB, __winx_hint_depth_bits,
		WGL_STENCIL_BITS_ARB, __winx_hint_stencil_bits,
		WGL_SAMPLE_BUFFERS_ARB, __winx_hint_multisamples ? 1 : 0,
		WGL_SAMPLES_ARB, __winx_hint_multisamples,
		0
	};

	int flags = 0;

	if (__winx_hint_opengl_debug) flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
	if (__winx_hint_opengl_robust) flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;

	int  contextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, __winx_hint_opengl_major,
		WGL_CONTEXT_MINOR_VERSION_ARB, __winx_hint_opengl_minor,
		WGL_CONTEXT_PROFILE_MASK_ARB, __winx_hint_opengl_core ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, flags,
		0
	};

	status = wglChoosePixelFormatARB(winx->device, pixelAttribs, NULL, 1, &pixelFormat, &numFormats);
	if (status && numFormats) {

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		DescribePixelFormat(winx->device, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

		if (!SetPixelFormat(winx->device, pixelFormat, &pfd)) {
			winxErrorMsg = (char*) "SetPixelFormat: Failed to select a pixel format!";
			return false;
		}

		winx->context = wglCreateContextAttribsARB(winx->device, 0, contextAttributes);
		if (!winx->context) {
			winxErrorMsg = (char*) "wglCreateContextAttribsARB: Failed to create a render context!";
			return false;
		}

	} else {
		winxErrorMsg = (char*) "wglChoosePixelFormatARB: Failed to choose a pixel format!";
		return false;
	}

	// close temporary window
	wglMakeCurrent(fakeDeviceContext, NULL);
	wglDeleteContext(fakeRenderContext);
	ReleaseDC(fakeHndl, fakeDeviceContext);
	DestroyWindow(fakeHndl);

	fakeDeviceContext = NULL;
	fakeRenderContext = NULL;
	fakeHndl = NULL;

	wglMakeCurrent(winx->device, winx->context);

	// set vsync
	winxSetVsync(__winx_hint_vsync);

	// finish window creation
	ShowWindow(winx->hndl, 1);
	UpdateWindow(winx->hndl);

	return true;
}

void winxPollEvents() {
	MSG event;

	while (PeekMessageA(&event, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&event);
		DispatchMessage(&event);
	}
}

void winxSwapBuffers() {
	SwapBuffers(winx->device);
}

void winxClose() {
	wglMakeCurrent(winx->device, NULL);
	wglDeleteContext(winx->context);
	ReleaseDC(winx->hndl, winx->device);
	DestroyWindow(winx->hndl);

	free(winx);
	winx = NULL;
}

void winxSetTitle(const char* title) {
	WINX_CONTEXT_ASSERT("winxSetTitle");

	SetWindowTextA(winx->hndl, title);
}

void winxSetIcon(int width, int height, unsigned char* buffer) {
	WINX_CONTEXT_ASSERT("winxSetIcon");

	HICON largeIcon, smallIcon;

	if (!buffer) {

		largeIcon = (HICON) GetClassLongPtrW(winx->hndl, GCLP_HICON);
        smallIcon = (HICON) GetClassLongPtrW(winx->hndl, GCLP_HICONSM);

	} else {

		char* target = NULL;
		BITMAPV5HEADER header = {0};

		header.bV5Size        = sizeof(header);
		header.bV5Width       = width;
		header.bV5Height      = -height;
		header.bV5Planes      = 1;
		header.bV5BitCount    = 32;
		header.bV5Compression = BI_BITFIELDS;

		// the order needs to be presisly like that
		// otherwise alpha doesn't work
		header.bV5RedMask     = 0x00ff0000;
		header.bV5GreenMask   = 0x0000ff00;
		header.bV5BlueMask    = 0x000000ff;
		header.bV5AlphaMask   = 0xff000000;

		HDC dc = GetDC(NULL);

		if (!dc) {
			winxErrorMsg = (char*) "GetDC: Failed to acquire drawable!";
			return;
		}

		HBITMAP mask = CreateBitmap(width, height, 1, 1, NULL);
		HBITMAP color = CreateDIBSection(dc, (BITMAPINFO*) &header, DIB_RGB_COLORS, (void**) &target, NULL, 0);
		ReleaseDC(NULL, dc);

		for (int i = 0; i < width * height;  i++) {
			target[0] = buffer[0];
			target[1] = buffer[1];
			target[2] = buffer[2];
			target[3] = buffer[3];

			target += 4;
			buffer += 4;
		}

		ICONINFO info = {0};
		info.fIcon = TRUE;
		info.xHotspot = 0;
		info.yHotspot = 0;
		info.hbmMask = mask;
		info.hbmColor = color;

		HICON icon = CreateIconIndirect(&info);

		DeleteObject(color);
		DeleteObject(mask);

		if (!icon) {
			winxErrorMsg = (char*) "CreateIconIndirect: Failed to create icon handle!";
			return;
		}

		largeIcon = icon;
		smallIcon = icon;

	}

	SendMessage(winx->hndl, WM_SETICON, ICON_BIG, (LPARAM) largeIcon);
	SendMessage(winx->hndl, WM_SETICON, ICON_SMALL, (LPARAM) smallIcon);
}

void winxSetVsync(int vsync) {
	WINX_CONTEXT_ASSERT("winxSetVsync");

	if (wglSwapIntervalEXT) {
		wglSwapIntervalEXT(__winx_hint_vsync);
	}
}

#endif // WINAPI

void winxSetMouseHandle(WinxMouseEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetMouseHandle");
	winx->mouse = handle;
}

void winxSetButtonEventHandle(WinxButtonEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetButtonEventHandle");
	winx->button = handle;
}

void winxSetKeybordEventHandle(WinxKeybordEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetKeybordEventHandle");
	winx->keyboard = handle;
}

void winxSetScrollEventHandle(WinxScrollEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetScrollEventHandle");
	winx->scroll = handle;
}

void winxSetCloseEventHandle(WinxCloseEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetCloseEventHandle");
	winx->close = handle;
}

void winxSetResizeEventHandle(WinxResizeEventHandle handle) {
	WINX_CONTEXT_ASSERT("winxSetResizeEventHandle");
	winx->resize = handle;
}

