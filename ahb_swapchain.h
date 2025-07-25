
#include <android/hardware_buffer.h>
#include <wlr/interfaces/wlr_buffer.h>

struct wlr_ahb_buffer {
	struct wlr_buffer base;
	struct wlr_dmabuf_attributes dmabuf;
	AHardwareBuffer *ahb;
	bool saved;
};

struct wlr_swapchain *wlr_swapchain_create_with_ahb(int width, int height);

struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer);