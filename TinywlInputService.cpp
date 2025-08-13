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


using SharedPtrType = std::shared_ptr<tinywl::TinywlInputService>;

AIBinder* TinywlInputService_asBinder(TinywlInputService service) {
  auto instance = static_cast<SharedPtrType *>(service)->get();
  return instance->asBinder().get();
}

TinywlInputService TinywlInputService_make() {
  auto service = ndk::SharedRefBase::make<tinywl::TinywlInputService>();
  
  //  Allocate a new shared_ptr on the heap and move the
  //  existing one into it. 
  SharedPtrType* shared_ptr_on_heap = new SharedPtrType(std::move(service));
  
  return static_cast<TinywlInputService>(shared_ptr_on_heap);
}

void TinywlInputService_setServer(TinywlInputService service, struct tinywl_server* server) {
  auto instance = static_cast<SharedPtrType *>(service)->get();
  instance->setTinywlServer(server);
}

struct wlr_keyboard TinywlInputService_getKeyboard(TinywlInputService service) {
  auto instance = static_cast<SharedPtrType *>(service)->get();
  return instance->keyboard;
}

struct wlr_pointer TinywlInputService_getPointer(TinywlInputService service) {
  auto instance = static_cast<SharedPtrType *>(service)->get();
  return instance->pointer;
}

void TinywlInputService_destroy(TinywlInputService service) {
    if (service) {
        // Reconstruct the shared_ptr to trigger its destructor
        auto shared_ptr = static_cast<SharedPtrType *>(service);
        delete shared_ptr;
    }
}
