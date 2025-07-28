#include "cutils/native_handle.h"
#include "vndk/hardware_buffer.h"
#include <android/hardware_buffer.h>
#include <assert.h>
#include <cstddef>
#include <linux/fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "ahb_swapchain.h"
#include "cros_gralloc_handle.h"
#include "cros_gralloc_util.h"

extern "C" {
	#include <wlr/util/log.h>
	#include <wlr/render/swapchain.h>
	#include <wlr/render/allocator.h>
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

struct wlr_buffer_impl ahb_buffer_impl = {
	.destroy = ahb_destroy,
	.get_dmabuf = ahb_get_dmabuf,
	// .begin_data_ptr_access = begin_data_ptr_access,
    // .end_data_ptr_access = end_data_ptr_access,
};


struct wlr_ahb_buffer *ahb_buffer_from_buffer(struct wlr_buffer *wlr_buffer) {
	assert(wlr_buffer->impl == &ahb_buffer_impl);
	struct wlr_ahb_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
	return buffer;
}



static struct wlr_ahb_buffer *create_wlr_ahb_buffer(uint32_t width, uint32_t height) {
	
	struct wlr_ahb_buffer *buffer = (wlr_ahb_buffer *)calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	AHardwareBuffer_Desc desc = {
        .width = width,
        .height = height,
        .layers = 1, // Single layer
        .format = AHB_FORMAT_PREFERRED, 
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | 
				AHARDWAREBUFFER_USAGE_COMPOSER_OVERLAY | 
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
    };

	AHardwareBuffer_allocate(&desc, &buffer->ahb);
	wlr_buffer_init(&buffer->base, &ahb_buffer_impl,
		width, height);
	
	const buffer_handle_t handle = AHardwareBuffer_getNativeHandle(buffer->ahb);
	
	if (!handle) {
		wlr_log(WLR_ERROR, "Failed to get handle from AHardwareBuffer");
		ahb_destroy(&buffer->base);
		return NULL;
	}
    cros_gralloc_handle* crosHandle = (cros_gralloc_handle*)handle;
	
	log_cros_gralloc_handle(crosHandle);
	buffer->dmabuf = cros_gralloc_to_wlr_dmabuf(crosHandle);
	// dmabuf->fd[0] = gralloc_handle(handle)->prime_fd;

	if (buffer->dmabuf.fd[0] < 0) {
		wlr_log(WLR_ERROR, "Failed to get dmabuf from AHardwareBuffer");
		ahb_destroy(&buffer->base);
		return NULL;
	}

	wlr_log(WLR_DEBUG, "wlr_dmabuf_attributes:");
	wlr_log(WLR_DEBUG, "  %dx%d, format: 0x%x, modifier: 0x%lx",
			buffer->dmabuf.width, buffer->dmabuf.height, buffer->dmabuf.format, buffer->dmabuf.modifier);
	for (int i = 0; i < buffer->dmabuf.n_planes; ++i) {
		wlr_log(WLR_DEBUG, "  Plane %d: fd=%d, stride=%u, offset=%u",
				i, buffer->dmabuf.fd[i], buffer->dmabuf.stride[i], buffer->dmabuf.offset[i]);
	}

	return buffer;
}

bool wlr_drm_format_copy(struct wlr_drm_format *dst, const struct wlr_drm_format *src) {
	assert(src->len <= src->capacity);

	uint64_t *modifiers = (uint64_t *)malloc(sizeof(*modifiers) * src->len);
	if (!modifiers) {
		return false;
	}

	memcpy(modifiers, src->modifiers, sizeof(*modifiers) * src->len);

	wlr_drm_format_finish(dst);
	dst->capacity = src->len;
	dst->len = src->len;
	dst->format = src->format;
	dst->modifiers = modifiers;
	return true;
}

struct wlr_swapchain *wlr_ahb_swapchain_create_for_output(struct wlr_output *output) {
	struct wlr_swapchain *swapchain = (struct wlr_swapchain *)calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->allocator = NULL;
	swapchain->width = output->width;
	swapchain->height = output->height;
	wlr_drm_format_copy(&swapchain->format, &output->swapchain->format);


	struct wlr_swapchain_slot *slot = &swapchain->slots[0];
	slot->acquired = false;
	slot->age = 0;
	slot->buffer = &create_wlr_ahb_buffer(output->width, output->height)->base;

	if (slot->buffer == NULL) {
		wlr_log(WLR_ERROR, "Failed to create AHB swapchain");
		free(swapchain);
		return NULL;
	}

	wlr_log(WLR_DEBUG, "AHB swapchain created");
	return swapchain;
}

