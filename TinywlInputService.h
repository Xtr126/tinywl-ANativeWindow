#include <android/binder_ibinder.h>

#include "tinywl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* TinywlInputService;

AIBinder* TinywlInputService_asBinder(TinywlInputService service);

void TinywlInputService_setServer(TinywlInputService service, struct tinywl_server* server);

struct wlr_keyboard TinywlInputService_getKeyboard(TinywlInputService service);

struct wlr_pointer TinywlInputService_getPointer(TinywlInputService service);

TinywlInputService TinywlInputService_make();

#ifdef __cplusplus
}
#endif