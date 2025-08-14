#pragma once

#include <android/binder_ibinder_jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "tinywl.h"

void TinywlInputService_setServer(struct tinywl_server* server);

void TinywlInputService_destroy();

#ifdef __cplusplus
}
#endif
