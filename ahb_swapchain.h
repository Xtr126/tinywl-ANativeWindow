
#include <android/hardware_buffer.h>
#include <wlr/interfaces/wlr_buffer.h>

#define AHB_FORMAT_PREFERRED AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM

struct wlr_ahb_buffer {
	struct wlr_buffer base;
	struct wlr_dmabuf_attributes dmabuf;
	AHardwareBuffer *ahb;
	bool saved;
};

struct wlr_swapchain *wlr_swapchain_create_with_ahb(int width, int height, const struct wlr_drm_format format, struct wlr_dmabuf_attributes *dmabuf);

struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer);