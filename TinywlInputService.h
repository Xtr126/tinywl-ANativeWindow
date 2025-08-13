#include <android/binder_ibinder_jni.h>

    
#ifdef __cplusplus
#include <memory>

// Forward declare the C++ class
namespace tinywl {
    class TinywlInputService;
}


#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* TinywlInputService;

#include "tinywl.h"

AIBinder* TinywlInputService_asBinder(TinywlInputService service);

void TinywlInputService_setServer(TinywlInputService service, struct tinywl_server* server);

struct wlr_keyboard TinywlInputService_getKeyboard(TinywlInputService service);

struct wlr_pointer TinywlInputService_getPointer(TinywlInputService service);

TinywlInputService TinywlInputService_make();

void TinywlInputService_destroy(TinywlInputService service);

#ifdef __cplusplus
}
#endif
