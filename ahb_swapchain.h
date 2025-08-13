
#include <android/hardware_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/types/wlr_output.h>


#include "ahb_wlr_allocator.h"

struct wlr_swapchain *wlr_ahb_swapchain_create_for_output(struct wlr_output *output);

struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer);

#ifdef __cplusplus
}
#endif