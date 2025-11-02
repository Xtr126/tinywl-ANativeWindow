#include <aidl/com/xtr/tinywl/BnTinywlSurface.h>
#include <android/binder_ibinder_jni.h>
#include "TinywlInputService.h"
#include "aidl/com/xtr/tinywl/TinywlXdgTopLevelCallback.h"
#include "aidl/com/xtr/tinywl/WlrBox.h"
#include "aidl/com/xtr/tinywl/XdgTopLevel.h"
#include "android/binder_auto_utils.h"
#include "android/binder_interface_utils.h"
#include "android/native_window.h"
#include "buffer_presenter.h"
#include <android/looper.h>
#include <assert.h>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
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

  static WlrBox WlrBox_from_wlr_box(struct wlr_box *wlr_box) {
    WlrBox wlrBox;
    wlrBox.x = wlr_box->x;
    wlrBox.y = wlr_box->y;
    wlrBox.width = wlr_box->width;
    wlrBox.height = wlr_box->height;
    return wlrBox;
  }

  class TinywlMainService : public BnTinywlSurface {
  public:
    ::ndk::ScopedAStatus onSurfaceChanged(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = reinterpret_cast<tinywl_toplevel *>(in_xdgToplevel.nativePtr);
      buffer_presenter_destroy(toplevel->buffer_presenter);
      return onSurfaceCreated(in_xdgToplevel);
    }

    ::ndk::ScopedAStatus onSurfaceCreated(const XdgTopLevel &in_xdgToplevel) override {
      ANativeWindow *window = in_xdgToplevel.surface.get();
      struct tinywl_toplevel *toplevel = reinterpret_cast<tinywl_toplevel *>(in_xdgToplevel.nativePtr);

      struct wlr_box* geo_box = &toplevel->geo_box;

      wlr_log(WLR_DEBUG, "Setting buffers geometry for ANativeWindow to %dx%d", geo_box->width, geo_box->height);
      int ret = ANativeWindow_setBuffersGeometry(window, geo_box->width, geo_box->height ,AHB_FORMAT_PREFERRED);
      if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to set buffers geometry: %s (%d)", strerror(-ret), -ret);
      }
      
      toplevel->buffer_presenter = buffer_presenter_create(window);

      mInputService->width = ANativeWindow_getWidth(window);
      mInputService->height = ANativeWindow_getWidth(window);

      return ::ndk::ScopedAStatus::ok();
    }

    ::ndk::ScopedAStatus onSurfaceDestroyed(const XdgTopLevel &in_xdgToplevel) override {
      struct tinywl_toplevel *toplevel = reinterpret_cast<tinywl_toplevel *>(in_xdgToplevel.nativePtr);
      buffer_presenter_destroy(toplevel->buffer_presenter);
      toplevel->buffer_presenter = nullptr;
      return ::ndk::ScopedAStatus::ok();
    }

    void registerXdgTopLevelCallback() {
        server.callbacks.data = this;

        server.callbacks.xdg_toplevel_add = [](struct tinywl_toplevel *toplevel, void* data) {
          auto thiz = reinterpret_cast<TinywlMainService *>(data);
          std::string appId;
          std::string title;
          if (toplevel->xdg_toplevel->app_id != NULL) appId = toplevel->xdg_toplevel->app_id; 
          if (toplevel->xdg_toplevel->title != NULL) title = toplevel->xdg_toplevel->title; 
          std::lock_guard<std::mutex> lock(thiz->mutex_);
          thiz->toplevels.insert(toplevel);
          thiz->mCallback->addXdgTopLevel(appId, title, (long)toplevel, WlrBox_from_wlr_box(&toplevel->geo_box));
        };

        server.callbacks.xdg_toplevel_remove = [](struct tinywl_toplevel *toplevel, void* data) {
          auto thiz = reinterpret_cast<TinywlMainService *>(data);
          std::lock_guard<std::mutex> lock(thiz->mutex_);
          thiz->toplevels.erase(toplevel);
          thiz->mCallback->removeXdgTopLevel(toplevel->xdg_toplevel->app_id, toplevel->xdg_toplevel->title, (long)toplevel);
        };
    }

    const std::shared_ptr<TinywlInputService> mInputService = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
    std::shared_ptr<ITinywlXdgTopLevelCallback> mCallback;

    struct tinywl_server server = {0};

  private:
      std::mutex &mutex_ = mInputService->mutex_;
      std::set<void *> &toplevels = mInputService->toplevels;
  };  // class TinywlMainService

}  // namespace tinywl

static auto gService = ndk::SharedRefBase::make<tinywl::TinywlMainService>();

extern "C"
JNIEXPORT void JNICALL
Java_com_xtr_tinywl_Tinywl_runTinywlLoop(JNIEnv *env, jclass clazz, jobjectArray args) {
  int argc = env->GetArrayLength(args);

  std::vector<std::string> argStrings;
  argStrings.reserve(argc);

  for (int i = 0; i < argc; i++) {
      jstring jstr = (jstring)env->GetObjectArrayElement(args, i);
      const char* utf = env->GetStringUTFChars(jstr, nullptr);
      argStrings.emplace_back(utf);
      env->ReleaseStringUTFChars(jstr, utf);
      env->DeleteLocalRef(jstr);
  }

  const char* startup_cmd = nullptr;

  for (int i = 0; i < argc; i++) {
      const std::string& arg = argStrings[i];

      if (arg == "-s") {
          if (i + 1 < argc) {
              startup_cmd = argStrings[++i].c_str();
          } else {
              std::cout << "Error: -s requires an argument\n";
              std::cout << "Usage: sh start.sh [-s startup command]\n";
              return;
          }
      } else if (arg == "-h") {
          std::cout << "Usage: sh start.sh [-s startup command]\n";
          return;
      } else {
          std::cout << "Usage: sh start.sh [-s startup command]\n";
          return;
      }
  }

  tinywl_init(1280, 720, &gService->server, startup_cmd);
  gService->mInputService->setTinywlServer(&gService->server);
  
  /* Handle event loop destroy */
  gService->mInputService->event_loop_destroy.notify = [](struct wl_listener *listener, void *data) {
    gService->mInputService->closeFdsAndRemoveEventSources();
  };
  
  struct wl_event_loop *loop = wl_display_get_event_loop(gService->server.wl_display);
  wl_event_loop_add_destroy_listener(loop, &gService->mInputService->event_loop_destroy);

  // Run loop
  tinywl_run_loop(&gService->server);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetTinywlInputServiceBinder(JNIEnv *env, jclass clazz) {
    return AIBinder_toJavaBinder(env, gService->mInputService->asBinder().get());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_tinywl_Tinywl_nativeGetTinywlSurfaceBinder(JNIEnv *env, jclass clazz) {
    return AIBinder_toJavaBinder(env, gService->asBinder().get());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_xtr_tinywl_Tinywl_nativeRegisterXdgTopLevelCallback(JNIEnv *env, jclass clazz,
                                                             jobject binder) {
    AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);
    const ::ndk::SpAIBinder spBinder(pBinder);
    gService->mCallback = aidl::com::xtr::tinywl::ITinywlXdgTopLevelCallback::fromBinder(spBinder);
    gService->registerXdgTopLevelCallback();
    wlr_log(WLR_INFO, "Registered callback");
}