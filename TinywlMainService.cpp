#include <aidl/com/xtr/tinywl/BnTinywlCallback.h>
#include <android/binder_ibinder_jni.h>
#include "Handler.h"
#include "TinywlInputService.h"
#include "aidl/com/xtr/tinywl/ITinywlXdgTopLevel.h"
#include "aidl/com/xtr/tinywl/XdgTopLevel.h"
#include "android/binder_auto_utils.h"
#include "android/native_window.h"
#include "buffer_presenter.h"
#include <android/looper.h>
#include <cassert>
#include <memory>
#include "ahb_wlr_allocator.h"

extern "C" {
  #include <wlr/util/log.h>
}

namespace tinywl {

  using namespace ::aidl::com::xtr::tinywl;

  static XdgTopLevel XdgTopLevel_from_wlr_xdg_toplevel(struct wlr_xdg_toplevel *xdg_toplevel) {
    XdgTopLevel xdgTopLevel;
    if (xdg_toplevel->app_id != nullptr) xdgTopLevel.appId = xdg_toplevel->app_id;
    if (xdg_toplevel->title != nullptr) xdgTopLevel.title = xdg_toplevel->title;
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
      return ::ndk::ScopedAStatus::ok();
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
      return ::ndk::ScopedAStatus::ok();
    }

    ::ndk::ScopedAStatus onSurfaceDestroyed(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = wl_container_of(in_xdgToplevel.nativePtr, toplevel, xdg_toplevel);
      buffer_presenter_destroy(toplevel->buffer_presenter);
      return ::ndk::ScopedAStatus::ok();
    }

    ::ndk::ScopedAStatus startTinywl(const std::shared_ptr<ITinywlXdgTopLevel> &in_xdgTopLevelCallback) override {
        this->xdgTopLevelCallback = in_xdgTopLevelCallback;
        server.callbacks.data = this;

        server.callbacks.xdg_toplevel_add = [](struct wlr_xdg_toplevel *xdg_toplevel, void* data) {
          auto service = reinterpret_cast<tinywl::TinywlMainService *>(data);
          service->xdgTopLevelCallback->addXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };

        server.callbacks.xdg_toplevel_remove = [](struct wlr_xdg_toplevel *xdg_toplevel, void* data) {
          auto service = reinterpret_cast<tinywl::TinywlMainService *>(data);
          service->xdgTopLevelCallback->removeXdgTopLevel(XdgTopLevel_from_wlr_xdg_toplevel(xdg_toplevel));
        };

        return ::ndk::ScopedAStatus::ok();
    }

    const std::shared_ptr<TinywlInputService> mInputService = TinywlInputService_make();

    struct tinywl_server server = {0};
    std::shared_ptr<ITinywlXdgTopLevel> xdgTopLevelCallback;

  private:
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
    return AIBinder_toJavaBinder(env, mService->asBinder().get());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xtr_tinywl_Tinywl_runTinywlLoop(JNIEnv *env, jclass clazz) {
  tinywl_init(1280, 720, &mService->server);
  mService->mInputService->setTinywlServer(&mService->server);
  tinywl_run_loop(&mService->server);
}