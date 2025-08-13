#include <aidl/tinywl/BnTinywlInput.h>
#include "aidl/com/android/server/inputflinger/KeyEventAction.h"

extern "C" {
  #include <wlr/interfaces/wlr_keyboard.h>
  #include <wlr/interfaces/wlr_pointer.h>
}

#include "TinywlInputService.h"


const struct wlr_pointer_impl tinywl_pointer_impl = {
    .name = "tinywl-pointer",
};

const struct wlr_keyboard_impl tinywl_keyboard_impl = {
    .name = "tinywl-keyboard",
};

namespace tinywl {
  using namespace aidl::com::android::server::inputflinger;
  using namespace aidl::android::hardware::input::common;

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
  private:
    struct tinywl_server* server;

  public:
    ::ndk::ScopedAStatus onKeyEvent(const KeyEvent& in_event, bool* _aidl_return) override {
      std::cout << in_event.toString() << std::endl;

      struct wlr_keyboard_key_event wlr_event = {
          .time_msec = (uint32_t)in_event.eventTime,
          .keycode = (uint32_t)in_event.scanCode,
          .update_state = true,
      };

      switch (in_event.action) {
        case KeyEventAction::DOWN:
            wlr_event.state = WL_KEYBOARD_KEY_STATE_PRESSED;
            break;
        case KeyEventAction::UP:
            wlr_event.state = WL_KEYBOARD_KEY_STATE_RELEASED;
            break;
        default:
            // Skip other actions like AKEY_EVENT_ACTION_MULTIPLE
            return ::ndk::ScopedAStatus::ok();;
      }

      wlr_keyboard_notify_key(&keyboard, &wlr_event);
      
      return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus onMotionEvent(const MotionEvent& in_event, bool* _aidl_return) override {
      std::cout << in_event.toString() << std::endl;
      return ::ndk::ScopedAStatus::ok();
    }

    void setTinywlServer(struct tinywl_server* server) {
      this->server = server;
      wlr_keyboard_init(&keyboard, &tinywl_keyboard_impl, "tinywl-keyboard");
      wlr_pointer_init(&pointer, &tinywl_pointer_impl, "tinywl-pointer");
    }

    struct wlr_keyboard keyboard;

    struct wlr_pointer pointer;
  };  // class TinywlInputService

}  // namespace tinywl


static std::shared_ptr<tinywl::TinywlInputService> g_shared_ptr;


void TinywlInputService_make() {
  g_shared_ptr = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
}

using instanceType = tinywl::TinywlInputService *;

AIBinder* TinywlInputService_asBinder() {
  return ndk::SharedRefBase::make<tinywl::TinywlInputService>()->asBinder().get();
}

void TinywlInputService_setServer(struct tinywl_server* server) {
  g_shared_ptr->setTinywlServer(server);
}

struct wlr_keyboard TinywlInputService_getKeyboard() {
  return g_shared_ptr->keyboard;
}

struct wlr_pointer TinywlInputService_getPointer() {
  return g_shared_ptr->pointer;
}

void TinywlInputService_destroy() {
    if (g_shared_ptr.get()) {
        delete g_shared_ptr.get();
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_xtr_compound_Tinywl_nativeGetBinder(JNIEnv *env, jclass clazz) {
    g_shared_ptr = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
    return AIBinder_toJavaBinder(env, g_shared_ptr->asBinder().get());
}