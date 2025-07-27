#include "cutils/native_handle.h"
#include "vndk/hardware_buffer.h"
#include <android/hardware_buffer.h>
#include <assert.h>
#include <linux/fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <wlr/render/swapchain.h>
#include <wlr/render/allocator.h>
#include "ahb_swapchain.h"

static const struct wlr_buffer_impl ahb_buffer_impl;


struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer) {
	assert(wlr_buffer->impl == &ahb_buffer_impl);
	struct wlr_ahb_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
	return buffer;
}


static void ahb_destroy(struct wlr_buffer *wlr_buffer) {
    struct wlr_ahb_buffer *buffer = ahb_buffer_from_buffer(wlr_buffer);
    AHardwareBuffer_release(buffer->ahb);
    free(buffer);
}

static bool ahb_get_dmabuf(struct wlr_buffer *wlr_buffer,
                              struct wlr_dmabuf_attributes *dmabuf) {
    struct wlr_ahb_buffer *buffer = ahb_buffer_from_buffer(wlr_buffer);
	*dmabuf = buffer->dmabuf;
    return true;
}

static const struct wlr_buffer_impl ahb_buffer_impl = {
	.destroy = ahb_destroy,
	.get_dmabuf = ahb_get_dmabuf,
	// .begin_data_ptr_access = begin_data_ptr_access,
    // .end_data_ptr_access = end_data_ptr_access,
};


static struct wlr_ahb_buffer *create_wlr_ahb_buffer(int width, int height, struct wlr_dmabuf_attributes *dmabuf) {
	
	struct wlr_ahb_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	AHardwareBuffer_Desc desc = {
        .width = width,
        .height = height,
        .layers = 1, // Single layer
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, 
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER 
    };

	AHardwareBuffer_allocate(&desc, &buffer->ahb);

	wlr_buffer_init(&buffer->base, &ahb_buffer_impl,
		width, height);
	
	const native_handle_t *handle = AHardwareBuffer_getNativeHandle(buffer->ahb);

    wlr_dmabuf_attributes_copy(&buffer->dmabuf, dmabuf);

	buffer->dmabuf.fd[0] = handle->data[0];
	
	return buffer;
}

struct wlr_swapchain *wlr_swapchain_create_with_ahb(int width, int height, const struct wlr_drm_format format, struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->allocator = NULL;
	swapchain->width = width;
	swapchain->height = height;
    swapchain->format = format;


	struct wlr_swapchain_slot *slot = &swapchain->slots[0];
	slot->acquired = false;
	slot->age = 0;
	slot->buffer = &create_wlr_ahb_buffer(width, height, dmabuf)->base;
	return swapchain;
}
