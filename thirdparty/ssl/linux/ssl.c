#include "x11_dummies.c"
#include "glbind_dummies.c"

#define SOKOL_APP_IMPL
#define SOKOL_GLCORE
#include "../../../thirdparty/sokol/sokol_app.h"
#define SOKOL_LOG_IMPL
#include "../../../thirdparty/sokol/sokol_log.h"
#define SOKOL_GFX_IMPL
#include "../../../thirdparty/sokol/sokol_gfx.h"
#define SOKOL_GLUE_IMPL
#include "../../../thirdparty/sokol/sokol_glue.h"
#define SOKOL_GL_IMPL
#include "../../../thirdparty/sokol/util/sokol_gl.h"

void ssl_init(){
    ssl_init_x11();
    ssl_init_glbind();
}