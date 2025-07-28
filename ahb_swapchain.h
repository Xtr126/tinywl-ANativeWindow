
#include <android/hardware_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/types/wlr_output.h>

#define AHB_FORMAT_PREFERRED AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM

struct wlr_ahb_buffer {
	struct wlr_buffer base;
	struct wlr_dmabuf_attributes dmabuf;
	AHardwareBuffer *ahb;
	bool saved;
};

struct wlr_swapchain *wlr_ahb_swapchain_create_with_dmabuf_attribs(struct wlr_dmabuf_attributes *dmabuf);
struct wlr_swapchain *wlr_ahb_swapchain_create_for_output(struct wlr_output *output);

struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer);

#ifdef __cplusplus
}
#endif