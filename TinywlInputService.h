#pragma once
#include "aidl/android/hardware/input/common/MotionEvent.h"
#include "aidl/com/android/server/inputflinger/KeyEvent.h"
#include <memory>
#include <wayland-server-core.h>
#ifdef __cplusplus

#include <aidl/tinywl/BnTinywlInput.h>
#include "TinywlInputServiceQueue.cpp"

extern "C" {
  #include "tinywl.h"
}

namespace tinywl {
  using namespace aidl::com::android::server::inputflinger;
  using namespace aidl::android::hardware::input::common;

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
    public:
      struct MotionEventTopLevel {
        std::unique_ptr<MotionEvent> motionEvent;
        long in_nativePtr;
      };
      int32_t width;
      int32_t height;
      ::ndk::ScopedAStatus onKeyEvent(const KeyEvent& in_event, long in_nativePtr, bool* _aidl_return) override;
      ::ndk::ScopedAStatus onMotionEvent(const MotionEvent& in_event, long in_nativePtr, bool* _aidl_return) override;
      void setTinywlServer(struct tinywl_server* server);
      void closeFdsAndRemoveEventSources();
      struct wlr_keyboard keyboard;
      struct wlr_pointer pointer;
      void sendPointerButtonEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      void sendPointerPosition(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      void sendScrollEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel);
      TinywlInputServiceQueue<MotionEvent> motionEventQueue;
      TinywlInputServiceQueue<KeyEvent> keyEventQueue;
      struct wl_listener event_loop_destroy;
      int event_fd;

    private:
      struct tinywl_server* server;
      struct wl_event_source *event_source;
  };
}  // namespace tinywl

#endif
