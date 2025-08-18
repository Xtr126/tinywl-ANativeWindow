#include <aidl/com/xtr/tinywl/BnTinywlCallback.h>
#include <android/binder_ibinder_jni.h>
#include "Handler.h"
#include "TinywlInputService.h"
#include "aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h"
#include "aidl/com/xtr/tinywl/XdgTopLevel.h"
#include "android/binder_auto_utils.h"
#include "android/binder_ibinder.h"
#include <android/looper.h>

extern "C" {
  #include "tinywl.h"
}

namespace tinywl {

  using namespace ::aidl::com::xtr::tinywl;

  static XdgTopLevel XdgTopLevel_from_wlr_xdg_toplevel(struct wlr_xdg_toplevel *xdg_toplevel) {
    XdgTopLevel xdgTopLevel;
    xdgTopLevel.appId = xdg_toplevel->app_id;
    xdgTopLevel.title = xdg_toplevel->title;
    xdgTopLevel.nativePtr = (long)xdg_toplevel;
    return xdgTopLevel;
  }


  class TinywlMainService : public BnTinywlCallback {
  public:
    ::ndk::ScopedAStatus onSurfaceChanged(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
    }

    ::ndk::ScopedAStatus onSurfaceCreated(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
    }

    ::ndk::ScopedAStatus onSurfaceDestroyed(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
    }

    ::ndk::ScopedAStatus startTinywl(const std::shared_ptr< ITinywlXdgTopLevel> &in_xdgTopLevelCallback) override {
        auto server = tinywl_init(in_xdgTopLevelCallback->asBinder().get());

        server.callbacks.xdg_toplevel_add = [](struct wlr_xdg_toplevel *xdg_toplevel, void* binder) {
          const ::ndk::SpAIBinder spBinder((AIBinder*)binder);
          auto xdgTopLevelCallback = ITinywlXdgTopLevel::fromBinder(spBinder);
          xdgTopLevelCallback->addXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };

        server.callbacks.xdg_toplevel_remove = [](struct wlr_xdg_toplevel *xdg_toplevel, void* binder) {
          const ::ndk::SpAIBinder spBinder((AIBinder*)binder);
          auto xdgTopLevelCallback = ITinywlXdgTopLevel::fromBinder(spBinder);
          xdgTopLevelCallback->removeXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };


        mHandler->post([server] {  
          // Run on main thread
          tinywl_run_loop(server);
        });
        
    }

    void setLooper(ALooper* looper) {
        mHandler = std::make_unique<Handler>(looper);
    }


  private:
    std::unique_ptr<Handler> mHandler;  
  };  // class TinywlMainService

}  // namespace tinywl



static std::shared_ptr<tinywl::TinywlMainService> mService;
static std::shared_ptr<tinywl::TinywlInputService> mInputService;

extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetInputServiceBinder(JNIEnv *env, jclass clazz) {
    mInputService = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
    return AIBinder_toJavaBinder(env, mInputService->asBinder().get());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetTinywlServiceBinder(JNIEnv *env, jclass clazz) {
    ALooper* mainLooper = ALooper_forThread();

    mService = ndk::SharedRefBase::make<tinywl::TinywlMainService>();
    mService->setLooper(mainLooper);

    return AIBinder_toJavaBinder(env, mService->asBinder().get());
}