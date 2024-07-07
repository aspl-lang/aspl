#ifndef ASPL_AILI_NO_STDLIB
#ifndef ASPL_AILI_BUNDLED // include all default stdlib implementations when manually invoking the interpreter
// TODO: This won't work as the implementation calls need certain wrappers
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#define aspl_ailinterpreter_dl_self NULL
#define aspl_ailinterpreter_dl_sym(handle, symbol) GetProcAddress(handle, symbol)
#else
#include <dlfcn.h>
#define aspl_ailinterpreter_dl_self RTLD_DEFAULT // TODO: This might not be available on all platforms
#define aspl_ailinterpreter_dl_sym(handle, symbol) dlsym(handle, symbol)
#endif

typedef ASPL_OBJECT_TYPE(*ASPL_AILI_ImplementationPtr)(ASPL_AILI_ArgumentList arguments);

ASPL_OBJECT_TYPE aspl_ailinterpreter_implement(ASPL_AILI_ThreadContext* thread_context, char* call, ASPL_AILI_ArgumentList args) {
    char* symbol = ASPL_MALLOC(strlen("aspl_ailinterpreter_implementation_") + strlen(call) + 1);
    strcat(symbol, "aspl_ailinterpreter_implementation_");
    int iBeforeCall = strlen(symbol);
    strcat(symbol, call);
    for (int i = iBeforeCall; i < strlen(symbol); i++) {
        if (symbol[i] == '.') {
            symbol[i] = '$';
        }
    }
    symbol[strlen("aspl_ailinterpreter_implementation_") + strlen(call)] = '\0';
    ASPL_AILI_ImplementationPtr cb = (ASPL_AILI_ImplementationPtr)aspl_ailinterpreter_dl_sym(aspl_ailinterpreter_dl_self, symbol);
    if (cb == NULL) {
        ASPL_PANIC("Cannot implement unknown implementation call %s", call);
    }
    return cb(args);
}