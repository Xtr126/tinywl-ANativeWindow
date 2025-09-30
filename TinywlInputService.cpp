#include "aidl/android/hardware/input/common/Axis.h"
#include <aidl/tinywl/BnTinywlInput.h>
#include <android/keycodes.h>
#include <cstdint>
#include <stdint.h>
#include <wayland-server-protocol.h>
#include "TinywlInputService.h"

extern "C" {
  #include <wlr/interfaces/wlr_keyboard.h>
  #include <wlr/interfaces/wlr_pointer.h>
  #include <linux/input-event-codes.h>
}

const struct wlr_pointer_impl tinywl_pointer_impl = {
    .name = "tinywl-pointer",
};

const struct wlr_keyboard_impl tinywl_keyboard_impl = {
    .name = "tinywl-keyboard",
};

extern float PointerCoords_getAxisValue(const aidl::android::hardware::input::common::PointerCoords& coords, int32_t axis);

namespace tinywl {

    void TinywlInputService::sendPointerButtonEvent(const MotionEvent& in_event, struct tinywl_toplevel *toplevel) {
         
      struct wlr_pointer_button_event wlr_event = {
          .pointer = &pointer,
          .time_msec = (uint32_t)in_event.eventTime,
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
      if (toplevel) surface = toplevel->xdg_toplevel->base->surface;

      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        server->seat->pointer_state.focused_surface = surface;
        server->seat->pointer_state.focused_client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        wlr_seat_pointer_send_button(server->seat, wlr_event.time_msec, wlr_event.button, wlr_event.state);
      }
    }

    void TinywlInputService::sendPointerPosition(const MotionEvent& in_event, struct tinywl_toplevel *toplevel) {
      float x = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::X));
      float y = PointerCoords_getAxisValue(in_event.pointerCoords.front(), static_cast<int32_t>(Axis::Y));
      struct wlr_seat_client *client = NULL;
      struct wlr_surface *surface;
      if (toplevel) surface = toplevel->xdg_toplevel->base->surface;

      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        struct wl_resource *resource;
        wl_resource_for_each(resource, &client->pointers) {
          if (wlr_seat_client_from_pointer_resource(resource) == NULL) {
            continue;
          }

          wl_pointer_send_motion(resource, in_event.eventTime, x, y);
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
      struct wlr_surface *surface = toplevel->xdg_toplevel->base->surface;
      if (surface) {
        struct wl_client *wl_client = wl_resource_get_client(surface->resource);
        server->seat->pointer_state.focused_surface = surface;
        server->seat->pointer_state.focused_client = wlr_seat_client_for_wl_client(server->seat, wl_client);
        wlr_seat_pointer_send_axis(server->seat, wlr_event.time_msec, wlr_event.orientation, wlr_event.delta, wlr_event.delta_discrete, wlr_event.source, wlr_event.relative_direction);
      }
    }

    ::ndk::ScopedAStatus TinywlInputService::onKeyEvent(const KeyEvent& in_event, long in_nativePtr, bool* _aidl_return) {
      // std::cout << in_event.toString() << std::endl;
      
      if (in_event.source != Source::KEYBOARD) {
        return ::ndk::ScopedAStatus::ok();
      }

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
      *_aidl_return = true;
      return ::ndk::ScopedAStatus::ok();
    }
    ::ndk::ScopedAStatus TinywlInputService::onMotionEvent(const MotionEvent& in_event, long in_nativePtr, bool* _aidl_return) {
      // std::cout << in_event.toString() << std::endl;
      
      if (in_event.source != Source::MOUSE) {
        return ::ndk::ScopedAStatus::ok();
      }

      switch (in_event.action) {
        case Action::BUTTON_PRESS:
        case Action::BUTTON_RELEASE:
          sendPointerButtonEvent(in_event, reinterpret_cast<struct tinywl_toplevel*>(in_nativePtr));
          break;
        case Action::MOVE:
        case Action::HOVER_ENTER:
        case Action::HOVER_EXIT:
          sendPointerPosition(in_event, reinterpret_cast<struct tinywl_toplevel*>(in_nativePtr));
          break;
        case Action::SCROLL:
          sendScrollEvent(in_event, reinterpret_cast<struct tinywl_toplevel*>(in_nativePtr));
          break;
        default:
            // Skip other actions
            return ::ndk::ScopedAStatus::ok();;
      }

      return ::ndk::ScopedAStatus::ok();
    }

    void TinywlInputService::setTinywlServer(struct tinywl_server* server) {
      this->server = server;
      wlr_keyboard_init(&keyboard, &tinywl_keyboard_impl, "tinywl-keyboard");
      server->new_input.notify(&server->new_input, &keyboard.base);

      wlr_pointer_init(&pointer, &tinywl_pointer_impl, "tinywl-pointer");
      server->new_input.notify(&server->new_input, &pointer.base);
    }

    int32_t width;
    int32_t height;

}  // namespace tinywl

std::shared_ptr<tinywl::TinywlInputService> TinywlInputService_make() { 
  return ndk::SharedRefBase::make<tinywl::TinywlInputService>();
}