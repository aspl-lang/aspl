#include <stdlib.h>
#include <dlfcn.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#ifndef SSL_ERROR_CB
#include <stdio.h>
#define SSL_ERROR_CB(...) { printf("SSL Error: "); printf(__VA_ARGS__); printf("\n"); exit(1); }
#endif

// dlopen the X11 libraries
char* x11Handle;
char* xcursorHandle;
char* xiHandle;
// dlsym the X11 functions
// XInput2.h
int (*XISelectEvents_ptr)(Display*, Window, XIEventMask*, int);
Status(*XIQueryVersion_ptr)(Display*, int*, int*);

// Xcursor.h
char* (*XcursorGetTheme_ptr)(Display*);
int (*XcursorGetDefaultSize_ptr)(Display*);
XcursorImage* (*XcursorLibraryLoadImage_ptr)(const char*, const char*, int);
Cursor(*XcursorLibraryLoadCursor_ptr)(Display*, const char*);
Cursor(*XcursorImageLoadCursor_ptr)(Display*, const XcursorImage*);
void (*XcursorImageDestroy_ptr)(XcursorImage*);
XcursorImage* (*XcursorImageCreate_ptr)(int, int);

// Xlib.h
Status(*XInitThreads_ptr)(void);
Status(*XrmInitialize_ptr)(void);
Display* (*XOpenDisplay_ptr)(const char*);
Bool(*XkbSetDetectableAutoRepeat_ptr)(Display*, Bool, Bool*);
int (*XFlush_ptr)(Display*);
int (*XPending_ptr)(Display*);
int (*XNextEvent_ptr)(Display*, XEvent*);
int (*XCloseDisplay_ptr)(Display*);
int (*XDefineCursor_ptr)(Display*, Window, Cursor);
int (*XUndefineCursor_ptr)(Display*, Window);
int (*XGrabPointer_ptr)(Display*, Window, Bool, unsigned int, int, int, Window, Cursor, Time);
int (*XWarpPointer_ptr)(Display*, Window, Window, int, int, unsigned int, unsigned int, int, int);
int (*XUngrabPointer_ptr)(Display*, Time);
void (*Xutf8SetWMProperties_ptr)(Display*, Window, const char*, const char*, char**, int, XSizeHints*, XWMHints*, XClassHint*);
int (*XChangeProperty_ptr)(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);
int (*XDeleteProperty_ptr)(Display*, Window, Atom);
char* (*XResourceManagerString_ptr)(Display*);
XrmDatabase(*XrmGetStringDatabase_ptr)(const char*);
Bool(*XrmGetResource_ptr)(XrmDatabase, const char*, const char*, char**, XrmValue*);
void (*XrmDestroyDatabase_ptr)(XrmDatabase);
Atom(*XInternAtom_ptr)(Display*, const char*, Bool);
Bool(*XQueryExtension_ptr)(Display*, const char*, int*, int*, int*);
int (*XFree_ptr)(void*);
Colormap(*XCreateColormap_ptr)(Display*, Window, Visual*, int);
Window(*XCreateWindow_ptr)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual*, unsigned long, XSetWindowAttributes*);
Status(*XSetWMProtocols_ptr)(Display*, Window, Atom*, int);
int (*XChangeGC_ptr)(Display*, GC, unsigned long, XGCValues*);
XSizeHints* (*XAllocSizeHints_ptr)(void);
void (*XSetWMNormalHints_ptr)(Display*, Window, XSizeHints*);
int (*XMapWindow_ptr)(Display*, Window);
int (*XRaiseWindow_ptr)(Display*, Window);
Bool(*XFilterEvent_ptr)(XEvent*, Window);
Bool(*XGetEventData_ptr)(Display*, XGenericEventCookie*);
void (*XFreeEventData_ptr)(Display*, XGenericEventCookie*);
int (*XLookupString_ptr)(XKeyEvent*, char*, int, KeySym*, XComposeStatus*);
int (*XConvertSelection_ptr)(Display*, Atom, Atom, Atom, Window, Time);
Status(*XSendEvent_ptr)(Display*, Window, Bool, long, XEvent*);
int (*XUnmapWindow_ptr)(Display*, Window);
int (*XDestroyWindow_ptr)(Display*, Window);
int (*XFreeColormap_ptr)(Display*, Colormap);
int (*XFreeCursor_ptr)(Display*, Cursor);
Cursor(*XCreateFontCursor_ptr)(Display*, unsigned int);
XErrorHandler(*XSetErrorHandler_ptr)(XErrorHandler);
int (*XSync_ptr)(Display*, Bool);
Status(*XGetWindowAttributes_ptr)(Display*, Window, XWindowAttributes*);
int (*XGetWindowProperty_ptr)(Display*, Window, Atom, long, long, Bool, Atom, Atom*, int*, unsigned long*, unsigned long*, unsigned char**);
KeySym* (*XGetKeyboardMapping_ptr)(Display*, KeyCode, int, int*);

void* check_dlsym_success(void* ptr) {
    if (ptr == NULL) {
        SSL_ERROR_CB("Could not load X11 function pointers: %s", dlerror());
    }
    return ptr;
}

void ssl_init_x11() {
    x11Handle = dlopen("libX11.so", RTLD_LAZY);
    if (!x11Handle) {
        SSL_ERROR_CB("Error while loading X11: %s", dlerror());
    }

    xcursorHandle = dlopen("libXcursor.so", RTLD_LAZY);
    if (!xcursorHandle) {
        SSL_ERROR_CB("Error while loading Xcursor: %s", dlerror());
    }

    xiHandle = dlopen("libXi.so", RTLD_LAZY);
    if (!xiHandle) {
        SSL_ERROR_CB("Error while loading XInput2: %s", dlerror());
    }

    XInitThreads_ptr = check_dlsym_success(dlsym(x11Handle, "XInitThreads"));
    XrmInitialize_ptr = check_dlsym_success(dlsym(x11Handle, "XrmInitialize"));
    XOpenDisplay_ptr = check_dlsym_success(dlsym(x11Handle, "XOpenDisplay"));
    XkbSetDetectableAutoRepeat_ptr = check_dlsym_success(dlsym(x11Handle, "XkbSetDetectableAutoRepeat"));
    XFlush_ptr = check_dlsym_success(dlsym(x11Handle, "XFlush"));
    XPending_ptr = check_dlsym_success(dlsym(x11Handle, "XPending"));
    XNextEvent_ptr = check_dlsym_success(dlsym(x11Handle, "XNextEvent"));
    XCloseDisplay_ptr = check_dlsym_success(dlsym(x11Handle, "XCloseDisplay"));
    XDefineCursor_ptr = check_dlsym_success(dlsym(x11Handle, "XDefineCursor"));
    XUndefineCursor_ptr = check_dlsym_success(dlsym(x11Handle, "XUndefineCursor"));
    XISelectEvents_ptr = check_dlsym_success(dlsym(xiHandle, "XISelectEvents"));
    XGrabPointer_ptr = check_dlsym_success(dlsym(x11Handle, "XGrabPointer"));
    XWarpPointer_ptr = check_dlsym_success(dlsym(x11Handle, "XWarpPointer"));
    XUngrabPointer_ptr = check_dlsym_success(dlsym(x11Handle, "XUngrabPointer"));
    Xutf8SetWMProperties_ptr = check_dlsym_success(dlsym(x11Handle, "Xutf8SetWMProperties"));
    XChangeProperty_ptr = check_dlsym_success(dlsym(x11Handle, "XChangeProperty"));
    XDeleteProperty_ptr = check_dlsym_success(dlsym(x11Handle, "XDeleteProperty"));
    XResourceManagerString_ptr = check_dlsym_success(dlsym(x11Handle, "XResourceManagerString"));
    XrmGetStringDatabase_ptr = check_dlsym_success(dlsym(x11Handle, "XrmGetStringDatabase"));
    XrmGetResource_ptr = check_dlsym_success(dlsym(x11Handle, "XrmGetResource"));
    XrmDestroyDatabase_ptr = check_dlsym_success(dlsym(x11Handle, "XrmDestroyDatabase"));
    XInternAtom_ptr = check_dlsym_success(dlsym(x11Handle, "XInternAtom"));
    XQueryExtension_ptr = check_dlsym_success(dlsym(x11Handle, "XQueryExtension"));
    XIQueryVersion_ptr = check_dlsym_success(dlsym(xiHandle, "XIQueryVersion"));
    XcursorGetTheme_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorGetTheme"));
    XcursorGetDefaultSize_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorGetDefaultSize"));
    XFree_ptr = check_dlsym_success(dlsym(x11Handle, "XFree"));
    XCreateColormap_ptr = check_dlsym_success(dlsym(x11Handle, "XCreateColormap"));
    XCreateWindow_ptr = check_dlsym_success(dlsym(x11Handle, "XCreateWindow"));
    XSetWMProtocols_ptr = check_dlsym_success(dlsym(x11Handle, "XSetWMProtocols"));
    XChangeGC_ptr = check_dlsym_success(dlsym(x11Handle, "XChangeGC"));
    XAllocSizeHints_ptr = check_dlsym_success(dlsym(x11Handle, "XAllocSizeHints"));
    XSetWMNormalHints_ptr = check_dlsym_success(dlsym(x11Handle, "XSetWMNormalHints"));
    XMapWindow_ptr = check_dlsym_success(dlsym(x11Handle, "XMapWindow"));
    XRaiseWindow_ptr = check_dlsym_success(dlsym(x11Handle, "XRaiseWindow"));
    XFilterEvent_ptr = check_dlsym_success(dlsym(x11Handle, "XFilterEvent"));
    XGetEventData_ptr = check_dlsym_success(dlsym(x11Handle, "XGetEventData"));
    XFreeEventData_ptr = check_dlsym_success(dlsym(x11Handle, "XFreeEventData"));
    XLookupString_ptr = check_dlsym_success(dlsym(x11Handle, "XLookupString"));
    XConvertSelection_ptr = check_dlsym_success(dlsym(x11Handle, "XConvertSelection"));
    XSendEvent_ptr = check_dlsym_success(dlsym(x11Handle, "XSendEvent"));
    XUnmapWindow_ptr = check_dlsym_success(dlsym(x11Handle, "XUnmapWindow"));
    XDestroyWindow_ptr = check_dlsym_success(dlsym(x11Handle, "XDestroyWindow"));
    XFreeColormap_ptr = check_dlsym_success(dlsym(x11Handle, "XFreeColormap"));
    XFreeCursor_ptr = check_dlsym_success(dlsym(x11Handle, "XFreeCursor"));
    XcursorLibraryLoadImage_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorLibraryLoadImage"));
    XcursorLibraryLoadCursor_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorLibraryLoadCursor"));
    XcursorImageLoadCursor_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorImageLoadCursor"));
    XcursorImageDestroy_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorImageDestroy"));
    XCreateFontCursor_ptr = check_dlsym_success(dlsym(x11Handle, "XCreateFontCursor"));
    XcursorImageCreate_ptr = check_dlsym_success(dlsym(xcursorHandle, "XcursorImageCreate"));
    XSetErrorHandler_ptr = check_dlsym_success(dlsym(x11Handle, "XSetErrorHandler"));
    XSync_ptr = check_dlsym_success(dlsym(x11Handle, "XSync"));
    XGetWindowAttributes_ptr = check_dlsym_success(dlsym(x11Handle, "XGetWindowAttributes"));
    XGetWindowProperty_ptr = check_dlsym_success(dlsym(x11Handle, "XGetWindowProperty"));
    XGetKeyboardMapping_ptr = check_dlsym_success(dlsym(x11Handle, "XGetKeyboardMapping"));
}

Status XInitThreads(
    void
) {
    return XInitThreads_ptr();
}

void XrmInitialize(
    void
) {
    XrmInitialize_ptr();
}

Display* XOpenDisplay(
    _Xconst char* display_name
) {
    return XOpenDisplay_ptr(display_name);
}

Bool XkbSetDetectableAutoRepeat(
    Display* dpy,
    Bool			 detectable,
    Bool* supported
) {
    return XkbSetDetectableAutoRepeat_ptr(dpy, detectable, supported);
}

int XFlush(
    Display* display
) {
    return XFlush_ptr(display);
}

int XPending(
    Display* display
) {
    return XPending_ptr(display);
}

int XNextEvent(
    Display* display,
    XEvent* event_return
) {
    return XNextEvent_ptr(display, event_return);
}

int XCloseDisplay(
    Display* display
) {
    return XCloseDisplay_ptr(display);
}

int XDefineCursor(
    Display* display,
    Window		 w,
    Cursor		 cursor
) {
    return XDefineCursor_ptr(display, w, cursor);
}

int XUndefineCursor(
    Display* display,
    Window		 w
) {
    return XUndefineCursor_ptr(display, w);
}

int XISelectEvents(
    Display* dpy,
    Window              win,
    XIEventMask* masks,
    int                 num_masks
) {
    return XISelectEvents_ptr(dpy, win, masks, num_masks);
}

int XGrabPointer(
    Display* display,
    Window		 grab_window,
    Bool		 owner_events,
    unsigned int	 event_mask,
    int			 pointer_mode,
    int			 keyboard_mode,
    Window		 confine_to,
    Cursor		 cursor,
    Time		 time
) {
    return XGrabPointer_ptr(display, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, time);
}

int XWarpPointer(
    Display* display,
    Window		 src_w,
    Window		 dest_w,
    int			 src_x,
    int			 src_y,
    unsigned int	 src_width,
    unsigned int	 src_height,
    int			 dest_x,
    int			 dest_y
) {
    return XWarpPointer_ptr(display, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y);
}

int XUngrabPointer(
    Display* display,
    Time		 time
) {
    return XUngrabPointer_ptr(display, time);
}

void Xutf8SetWMProperties(
    Display* display,
    Window		 w,
    _Xconst char* window_name,
    _Xconst char* icon_name,
    char** argv,
    int			 argc,
    XSizeHints* normal_hints,
    XWMHints* wm_hints,
    XClassHint* class_hints
) {
    Xutf8SetWMProperties_ptr(display, w, window_name, icon_name, argv, argc, normal_hints, wm_hints, class_hints);
}

int XChangeProperty(
    Display* display,
    Window		 w,
    Atom		 property,
    Atom		 type,
    int			 format,
    int			 mode,
    _Xconst unsigned char* data,
    int			 nelements
) {
    return XChangeProperty_ptr(display, w, property, type, format, mode, data, nelements);
}

int XDeleteProperty(
    Display* display,
    Window		 w,
    Atom		 property
) {
    return XDeleteProperty_ptr(display, w, property);
}

int XSetWMProtocols(
    Display* display,
    Window		 w,
    Atom* protocols,
    int			 count
) {
    return XSetWMProtocols_ptr(display, w, protocols, count);
}

int XChangeGC(
    Display* display,
    GC			 gc,
    unsigned long	 valuemask,
    XGCValues* values
) {
    return XChangeGC_ptr(display, gc, valuemask, values);
}

int XGetWindowProperty(
    Display* display,
    Window		 w,
    Atom		 property,
    long		 long_offset,
    long		 long_length,
    Bool		 delete,
    Atom		 req_type,
    Atom* actual_type_return,
    int* actual_format_return,
    unsigned long* nitems_return,
    unsigned long* bytes_after_return,
    unsigned char** prop_return
) {
    return XGetWindowProperty_ptr(display, w, property, long_offset, long_length, delete, req_type, actual_type_return, actual_format_return, nitems_return, bytes_after_return, prop_return);
}

int XFree(
    void* data
) {
    return XFree_ptr(data);
}

Atom XInternAtom(
    Display* display,
    _Xconst char* atom_name,
    Bool		 only_if_exists
) {
    return XInternAtom_ptr(display, atom_name, only_if_exists);
}

char* XResourceManagerString(
    Display* display
) {
    return XResourceManagerString_ptr(display);
}

XrmDatabase XrmGetStringDatabase(
    _Xconst char* data   // null terminated string
) {
    return XrmGetStringDatabase_ptr(data);
}

Bool XrmGetResource(
    XrmDatabase		database,
    _Xconst char* str_name,
    _Xconst char* str_class,
    char** str_type_return,
    XrmValue* value_return
) {
    return XrmGetResource_ptr(database, str_name, str_class, str_type_return, value_return);
}

void XrmDestroyDatabase(
    XrmDatabase		database
) {
    XrmDestroyDatabase_ptr(database);
}

Bool XQueryExtension(
    Display* display,
    _Xconst char* name,
    int* major_opcode_return,
    int* first_event_return,
    int* first_error_return
) {
    return XQueryExtension_ptr(display, name, major_opcode_return, first_event_return, first_error_return);
}

Status XIQueryVersion(
    Display* dpy,
    int* major_version_inout,
    int* minor_version_inout
) {
    return XIQueryVersion_ptr(dpy, major_version_inout, minor_version_inout);
}

char*
XcursorGetTheme(Display* dpy)
{
    return XcursorGetTheme_ptr(dpy);
}

int
XcursorGetDefaultSize(Display* dpy)
{
    return XcursorGetDefaultSize_ptr(dpy);
}

Colormap XCreateColormap(
    Display* display,
    Window		w,
    Visual* visual,
    int			alloc
) {
    return XCreateColormap_ptr(display, w, visual, alloc);
}

Window XCreateWindow(
    Display* display,
    Window		parent,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    unsigned int	border_width,
    int			depth,
    unsigned int	class,
    Visual* visual,
    unsigned long	valuemask,
    XSetWindowAttributes* attributes
) {
    return XCreateWindow_ptr(display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes);
}

XSizeHints* XAllocSizeHints(
    void
) {
    return XAllocSizeHints_ptr();
}

void XSetWMNormalHints(
    Display* display,
    Window		w,
    XSizeHints* hints
) {
    XSetWMNormalHints_ptr(display, w, hints);
}

int XMapWindow(
    Display* display,
    Window		w
) {
    return XMapWindow_ptr(display, w);
}

int XRaiseWindow(
    Display* display,
    Window		w
) {
    return XRaiseWindow_ptr(display, w);
}

Bool XFilterEvent(
    XEvent* event,
    Window	window
) {
    return XFilterEvent_ptr(event, window);
}

Bool XGetEventData(
    Display* dpy,
    XGenericEventCookie* cookie
) {
    return XGetEventData_ptr(dpy, cookie);
}

void XFreeEventData(
    Display* dpy,
    XGenericEventCookie* cookie
) {
    XFreeEventData_ptr(dpy, cookie);
}

int XLookupString(
    XKeyEvent* event_struct,
    char* buffer_return,
    int			bytes_buffer,
    KeySym* keysym_return,
    XComposeStatus* status_in_out
) {
    return XLookupString_ptr(event_struct, buffer_return, bytes_buffer, keysym_return, status_in_out);
}

int XConvertSelection(
    Display* display,
    Atom		selection,
    Atom 		target,
    Atom		property,
    Window		requestor,
    Time		time
) {
    return XConvertSelection_ptr(display, selection, target, property, requestor, time);
}

Status XSendEvent(
    Display* display,
    Window		w,
    Bool		propagate,
    long		event_mask,
    XEvent* event_send
) {
    return XSendEvent_ptr(display, w, propagate, event_mask, event_send);
}

int XUnmapWindow(
    Display* display,
    Window		w
) {
    return XUnmapWindow_ptr(display, w);
}

int XDestroyWindow(
    Display* display,
    Window		w
) {
    return XDestroyWindow_ptr(display, w);
}

int XFreeColormap(
    Display* display,
    Colormap		colormap
) {
    return XFreeColormap_ptr(display, colormap);
}

int XFreeCursor(
    Display* display,
    Cursor		cursor
) {
    return XFreeCursor_ptr(display, cursor);
}

XcursorImage*
XcursorLibraryLoadImage(const char* library, const char* theme, int size)
{
    return XcursorLibraryLoadImage_ptr(library, theme, size);
}

Cursor
XcursorLibraryLoadCursor(Display* dpy, const char* file)
{
    return XcursorLibraryLoadCursor_ptr(dpy, file);
}

Cursor
XcursorImageLoadCursor(Display* dpy, const XcursorImage* image)
{
    return XcursorImageLoadCursor_ptr(dpy, image);
}

void
XcursorImageDestroy(XcursorImage* image)
{
    XcursorImageDestroy_ptr(image);
}

Cursor XCreateFontCursor(
    Display* display,
    unsigned int	shape
) {
    return XCreateFontCursor_ptr(display, shape);
}

XcursorImage*
XcursorImageCreate(int width, int height) {
    return XcursorImageCreate_ptr(width, height);
}

XErrorHandler XSetErrorHandler(
    XErrorHandler	x11Handler
) {
    return XSetErrorHandler_ptr(x11Handler);
}

int XSync(
    Display* display,
    Bool		discard
) {
    return XSync_ptr(display, discard);
}

Status XGetWindowAttributes(
    Display* display,
    Window		w,
    XWindowAttributes* window_attributes_return
) {
    return XGetWindowAttributes_ptr(display, w, window_attributes_return);
}

KeySym* XGetKeyboardMapping(
    Display* display,
#if NeedWidePrototypes
    unsigned int	first_keycode,
#else
    KeyCode		first_keycode,
#endif
    int			keycode_count,
    int* keysyms_per_keycode_return
) {
    return XGetKeyboardMapping_ptr(display, first_keycode, keycode_count, keysyms_per_keycode_return);
}