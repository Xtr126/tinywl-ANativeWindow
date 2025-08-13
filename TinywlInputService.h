#include <android/binder_ibinder.h>

    
#ifdef __cplusplus
#include <memory>

// Forward declare the C++ class
namespace tinywl {
    class TinywlInputService;
}

// Define the smart pointer type we will be using internally
using TinywlInputServiceSharedRef = std::shared_ptr<tinywl::TinywlInputService>;

// Use element_type to get the raw pointer type.
// This is the underlying type of the object we manage.
using TinywlInputServiceRawPtr = TinywlInputServiceSharedRef::element_type*;

// We will return the smart pointer itself as the opaque handle.
// This is the safest way to ensure lifetime management.
using TinywlInputServiceOpaqueType = TinywlInputServiceSharedRef;

// The C-facing typedef
typedef TinywlInputServiceOpaqueType TinywlInputService;

#else

// C-facing definition
typedef void* TinywlInputService;

#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tinywl.h"

AIBinder* TinywlInputService_asBinder(TinywlInputService service);

void TinywlInputService_setServer(TinywlInputService service, struct tinywl_server* server);

struct wlr_keyboard TinywlInputService_getKeyboard(TinywlInputService service);

struct wlr_pointer TinywlInputService_getPointer(TinywlInputService service);

TinywlInputService TinywlInputService_make();

#ifdef __cplusplus
}
#endif
