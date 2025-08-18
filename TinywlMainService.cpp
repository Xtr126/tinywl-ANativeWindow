#include <aidl/com/xtr/tinywl/BnTinywlCallback.h>
#include <android/binder_ibinder_jni.h>
#include "Handler.h"
#include "TinywlInputService.h"
#include "aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h"
#include "aidl/com/xtr/tinywl/XdgTopLevel.h"
#include "android/binder_auto_utils.h"
#include "android/binder_ibinder.h"
#include "android/native_window.h"
#include "buffer_presenter.h"
#include <android/looper.h>
#include "ahb_wlr_allocator.h"

extern "C" {
  #include <wlr/util/log.h>
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
      ANativeWindow *window = in_xdgToplevel.surface.get();
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
      buffer_presenter_destroy(toplevel->buffer_presenter);
      toplevel->buffer_presenter = buffer_presenter_create(window);
      // 	TinywlInputService_onWindowResize(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
      mInputService->width = ANativeWindow_getWidth(window);
    }

    ::ndk::ScopedAStatus onSurfaceCreated(const XdgTopLevel &in_xdgToplevel) override {
      ANativeWindow *window = in_xdgToplevel.surface.get();
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
      struct wlr_buffer buffer = toplevel->xdg_toplevel->base->surface->buffer->base;

      wlr_log(WLR_DEBUG, "Setting buffers geometry for ANativeWindow to %dx%d", buffer.width, buffer.height);
      int ret = ANativeWindow_setBuffersGeometry(window, buffer.width, buffer.height ,AHB_FORMAT_PREFERRED);
      if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to set buffers geometry: %s (%d)", strerror(-ret), -ret);
      }
      
      toplevel->buffer_presenter = buffer_presenter_create(window);
    }

    ::ndk::ScopedAStatus onSurfaceDestroyed(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
      buffer_presenter_destroy(toplevel->buffer_presenter);
    }

    ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<ITinywlXdgTopLevel> &in_xdgTopLevelCallback) override {
        auto server = tinywl_init(1280, 1280);

        server.callbacks.data = in_xdgTopLevelCallback->asBinder().get();

        server.callbacks.xdg_toplevel_add = [](struct wlr_xdg_toplevel *xdg_toplevel, void* data) {
          const ::ndk::SpAIBinder spBinder((AIBinder*)data);
          auto xdgTopLevelCallback = ITinywlXdgTopLevel::fromBinder(spBinder);
          xdgTopLevelCallback->addXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };

        server.callbacks.xdg_toplevel_remove = [](struct wlr_xdg_toplevel *xdg_toplevel, void* data) {
          const ::ndk::SpAIBinder spBinder((AIBinder*)data);
          auto xdgTopLevelCallback = ITinywlXdgTopLevel::fromBinder(spBinder);
          xdgTopLevelCallback->removeXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };

        mInputService->setTinywlServer(&server);

        mHandler->post([server] {  
          // Run on main thread
          tinywl_run_loop(server);
        });
    }

    void setLooper(ALooper* looper) {
        mHandler = std::make_unique<Handler>(looper);
    }

    const std::shared_ptr<TinywlInputService> mInputService = TinywlInputService_make();

  private:
    std::unique_ptr<Handler> mHandler;  
  };  // class TinywlMainService

}  // namespace tinywl

static auto mService = ndk::SharedRefBase::make<tinywl::TinywlMainService>();

extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetInputServiceBinder(JNIEnv *env, jclass clazz) {
    return AIBinder_toJavaBinder(env, mService->mInputService->asBinder().get());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetTinywlServiceBinder(JNIEnv *env, jclass clazz) {
    ALooper* mainLooper = ALooper_forThread();
    mService->setLooper(mainLooper);

    return AIBinder_toJavaBinder(env, mService->asBinder().get());
}