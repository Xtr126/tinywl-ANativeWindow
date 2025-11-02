#include "aidl/android/hardware/input/common/Axis.h"
#include "aidl/android/hardware/input/common/MotionEvent.h"
#include "aidl/com/android/server/inputflinger/KeyEvent.h"
#include <aidl/tinywl/BnTinywlInput.h>
#include <android/keycodes.h>
#include <cstdint>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include "TinywlInputService.h"
#include <sys/eventfd.h>

extern "C" {
  #include <wlr/interfaces/wlr_keyboard.h>
  #include <wlr/interfaces/wlr_pointer.h>
  #include <linux/input-event-codes.h>
  #include <wlr/util/log.h>
}

extern float PointerCoords_getAxisValue(const aidl::android::hardware::input::common::PointerCoords& coords, int32_t axis);

namespace tinywl {

    const static struct wlr_pointer_impl tinywl_pointer_impl = {
        .name = "tinywl-pointer",
    };

    const static struct wlr_keyboard_impl tinywl_keyboard_impl = {
        .name = "tinywl-keyboard",
    };


    void TinywlInputService::sendPointerButtonEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel) {
      struct wlr_pointer_button_event wlr_event = {
          .pointer = &pointer,
          .time_msec = static_cast<uint32_t>(in_event.eventTime),
          .state = WL_POINTER_BUTTON_STATE_PRESSED,
      };

      switch (in_event.action) {
        case Action::BUTTON_PRESS:
            wlr_event.state = WL_POINTER_BUTTON_STATE_PRESSED;
            break;
        case Action::BUTTON_RELEASE:
            wlr_event.state = WL_POINTER_BUTTON_STATE_RELEASED;
            break;
        default:
            return;
      }

      switch (in_event.actionButton) {
        case Button::PRIMARY:
            wlr_event.button = BTN_LEFT;
            break;
        case Button::SECONDARY:
            wlr_event.button = BTN_RIGHT;
            break;
        default:
            return;
      }

      struct wlr_surface *surface;
      if (toplevel && toplevel->xdg_toplevel)
        surface = toplevel->xdg_toplevel->base->surface;

      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        server->seat->pointer_state.focused_surface = surface;
        server->seat->pointer_state.focused_client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        wlr_seat_pointer_send_button(server->seat, wlr_event.time_msec, wlr_event.button, wlr_event.state);
        wl_signal_emit_mutable(&pointer.events.frame, &pointer);
      }
    }

    void TinywlInputService::sendPointerPosition(const MotionEvent& in_event, struct tinywl_toplevel *toplevel) {
      float x = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::X));
      float y = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::Y));
      struct wlr_seat_client *client = NULL;
      struct wlr_surface *surface;
      if (toplevel && toplevel->xdg_toplevel)
        surface = toplevel->xdg_toplevel->base->surface;

      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        struct wl_resource *resource;
        wl_resource_for_each(resource, &client->pointers) {
          if (wlr_seat_client_from_pointer_resource(resource) == NULL) {
            continue;
          }

          wl_pointer_send_motion(resource, in_event.eventTime, x, y);
          wl_signal_emit_mutable(&pointer.events.frame, &pointer);
        }
      }
    }

    void TinywlInputService::sendScrollEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel) {
      float delta = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::Y));
      struct wlr_pointer_axis_event wlr_event = {
        .pointer = &pointer,
        .time_msec = static_cast<uint32_t>(in_event.eventTime),
        .source = WL_POINTER_AXIS_SOURCE_WHEEL,
        .orientation = WL_POINTER_AXIS_VERTICAL_SCROLL,
        .delta = delta,
        .delta_discrete = static_cast<int32_t>(delta * WLR_POINTER_AXIS_DISCRETE_STEP),
      };
      struct wlr_surface *surface;
      if (toplevel && toplevel->xdg_toplevel)
        surface = toplevel->xdg_toplevel->base->surface;

      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        server->seat->pointer_state.focused_surface = surface;
        server->seat->pointer_state.focused_client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        wlr_seat_pointer_send_axis(server->seat, wlr_event.time_msec, wlr_event.orientation, wlr_event.delta, wlr_event.delta_discrete, wlr_event.source, wlr_event.relative_direction);
        wl_signal_emit_mutable(&pointer.events.frame, &pointer);
      }
    }

    ::ndk::ScopedAStatus TinywlInputService::onKeyEvent(const KeyEvent& in_event, long in_nativePtr, bool* _aidl_return) {
      keyEventQueue.push(in_event, in_nativePtr);
      eventfd_write(event_fd, 1);  // Notify event loop
      // std::cout << in_event.toString() << std::endl;
      *_aidl_return = true;
      return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus TinywlInputService::onMotionEvent(const MotionEvent& in_event, long in_nativePtr, bool* _aidl_return) {
      motionEventQueue.push(in_event, in_nativePtr);
      eventfd_write(event_fd, 1);  // Notify event loop
      // std::cout << in_event.toString() << std::endl;
      *_aidl_return = true;
      return ::ndk::ScopedAStatus::ok();
    }

    static int event_loop_fd_callback(int fd, uint32_t mask, void *data) {
        auto thiz = reinterpret_cast<TinywlInputService *>(data);
        if ((mask & WL_EVENT_HANGUP) || (mask & WL_EVENT_ERROR)) {
            if (mask & WL_EVENT_ERROR) {
                wlr_log(WLR_ERROR, "Failed to read event");
            }
            return 0;
        }

        eventfd_t event_count = 0;
        if (eventfd_read(thiz->event_fd, &event_count) < 0) {
            return 0;
        }

        while (auto in_event = thiz->motionEventQueue.pull()) {
          // Process event
          if (in_event->event.source != Source::MOUSE) {
            continue;
          }

          switch (in_event->event.action) {
            case Action::BUTTON_PRESS:
            case Action::BUTTON_RELEASE:
              thiz->sendPointerButtonEvent(in_event->event, reinterpret_cast<struct tinywl_toplevel*>(in_event->nativePtr));
              break;
            case Action::MOVE:
            case Action::HOVER_ENTER:
            case Action::HOVER_EXIT:
              thiz->sendPointerPosition(in_event->event, reinterpret_cast<struct tinywl_toplevel*>(in_event->nativePtr));
              break;
            case Action::SCROLL:
              thiz->sendScrollEvent(in_event->event, reinterpret_cast<struct tinywl_toplevel*>(in_event->nativePtr));
              break;
            default:
                // Skip other actions
                continue;
          }
        }

        while (auto in_event = thiz->keyEventQueue.pull()) {
          // Process event
          if (in_event->event.source != Source::KEYBOARD) {
            continue;
          }

          struct wlr_keyboard_key_event wlr_event = {
              .time_msec = static_cast<uint32_t>(get_current_time_msec()),
              .keycode = (uint32_t)in_event->event.scanCode,
              .update_state = true,
          };

          switch (in_event->event.action) {
            case KeyEventAction::DOWN:
                wlr_event.state = WL_KEYBOARD_KEY_STATE_PRESSED;
                break;
            case KeyEventAction::UP:
                wlr_event.state = WL_KEYBOARD_KEY_STATE_RELEASED;
                break;
            default:
                // Skip other actions like AKEY_EVENT_ACTION_MULTIPLE
                continue;
          }

          wlr_keyboard_notify_key(&thiz->keyboard, &wlr_event);
        }
        return 0;
    }

    void TinywlInputService::setTinywlServer(struct tinywl_server* server) {
      this->server = server;
      wlr_keyboard_init(&keyboard, &tinywl_keyboard_impl, "tinywl-keyboard");
      server->new_input.notify(&server->new_input, &keyboard.base);

      wlr_pointer_init(&pointer, &tinywl_pointer_impl, "tinywl-pointer");
      server->new_input.notify(&server->new_input, &pointer.base);
      
      /* Event loop and file descriptors */
      struct wl_event_loop *loop = wl_display_get_event_loop(server->wl_display);
      event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);

      uint32_t events = WL_EVENT_READABLE | WL_EVENT_ERROR | WL_EVENT_HANGUP;
      event_source = wl_event_loop_add_fd(loop, event_fd, events, event_loop_fd_callback, this);

    }

    void TinywlInputService::closeFdsAndRemoveEventSources() {
      wl_event_source_remove(event_source);
      close(event_fd);
    }
} // namespace tinywl

