#include "cutils/native_handle.h"
#include "vndk/hardware_buffer.h"
#include <android/hardware_buffer.h>
#include <assert.h>
#include <linux/fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "ahb_swapchain.h"
#include "gralloc_handle.h"
#include "cros_gralloc_handle.h"

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

static const struct wlr_buffer_impl ahb_buffer_impl = {
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

void log_cros_gralloc_handle(const struct cros_gralloc_handle *handle) {
	wlr_log(WLR_DEBUG, "=== cros_gralloc_handle ===");
	wlr_log(WLR_DEBUG, "id: %u", handle->id);
	wlr_log(WLR_DEBUG, "width: %u, height: %u", handle->width, handle->height);
	wlr_log(WLR_DEBUG, "format: 0x%x (DRM), droid_format: %d", handle->format, handle->droid_format);
	wlr_log(WLR_DEBUG, "tiling: %u", handle->tiling);
	wlr_log(WLR_DEBUG, "format_modifier: 0x%lx", handle->format_modifier);
	wlr_log(WLR_DEBUG, "use_flags: 0x%lx", handle->use_flags);
	wlr_log(WLR_DEBUG, "magic: 0x%x", handle->magic);
	wlr_log(WLR_DEBUG, "pixel_stride: %u", handle->pixel_stride);
	wlr_log(WLR_DEBUG, "usage: 0x%lx", handle->usage);
	wlr_log(WLR_DEBUG, "num_planes: %u", handle->num_planes);
	wlr_log(WLR_DEBUG, "reserved_region_size: %lu", handle->reserved_region_size);
	wlr_log(WLR_DEBUG, "total_size: %lu", handle->total_size);

	for (uint32_t i = 0; i < DRV_MAX_PLANES; ++i) {
		wlr_log(WLR_DEBUG, "Plane %u:", i);
		wlr_log(WLR_DEBUG, "  fd: %d", handle->fds[i]);
		wlr_log(WLR_DEBUG, "  stride: %u", handle->strides[i]);
		wlr_log(WLR_DEBUG, "  offset: %u", handle->offsets[i]);
		wlr_log(WLR_DEBUG, "  size: %u", handle->sizes[i]);
	}

	if (handle->reserved_region_size > 0) {
		int meta_fd_index = handle->num_planes;
		wlr_log(WLR_DEBUG, "Metadata reserved region FD: %d", handle->fds[meta_fd_index]);
	}

	wlr_log(WLR_DEBUG, "=== End of cros_gralloc_handle ===");
}

bool cros_gralloc_to_wlr_dmabuf(const struct cros_gralloc_handle *handle,
                                 struct wlr_dmabuf_attributes *attribs) {
	const int MAX_PLANES = DRV_MAX_PLANES;

	if (handle == NULL || attribs == NULL) {
		return false;
	}

	// Sanity check: avoid using bad num_planes
	int num_planes = handle->num_planes;
	if (num_planes <= 0 || num_planes > MAX_PLANES) {
		wlr_log(WLR_ERROR, "Invalid num_planes: %d, clamping to %d", num_planes, MAX_PLANES);
		num_planes = MAX_PLANES;
	}

	attribs->width = handle->width;
	attribs->height = handle->height;
	attribs->format = handle->format;
	attribs->modifier = handle->format_modifier;
	attribs->n_planes = 1;

	for (int i = 0; i < num_planes; i++) {
		attribs->fd[i] = handle->fds[i];
		attribs->stride[i] = handle->strides[i];
		attribs->offset[i] = handle->offsets[i];
	}

	return true;
}


static struct wlr_ahb_buffer *create_wlr_ahb_buffer(struct wlr_dmabuf_attributes *dmabuf) {
	
	struct wlr_ahb_buffer *buffer = (wlr_ahb_buffer *)calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	AHardwareBuffer_Desc desc = {
        .width = static_cast<uint32_t>(dmabuf->width),
        .height = static_cast<uint32_t>(dmabuf->height),
        .layers = 1, // Single layer
        .format = AHB_FORMAT_PREFERRED, 
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | 
				AHARDWAREBUFFER_USAGE_COMPOSER_OVERLAY | 
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
    };

	AHardwareBuffer_allocate(&desc, &buffer->ahb);
	wlr_buffer_init(&buffer->base, &ahb_buffer_impl,
		dmabuf->width, dmabuf->height);
	
	const buffer_handle_t handle = AHardwareBuffer_getNativeHandle(buffer->ahb);
	
	if (!handle) {
		wlr_log(WLR_ERROR, "Failed to get handle from AHardwareBuffer");
		ahb_destroy(&buffer->base);
		return NULL;
	}

	if (!gralloc_handle(handle)) {
		wlr_log(WLR_ERROR, "Not drm/gbm gralloc");
		ahb_destroy(&buffer->base);
		return NULL;
	}

    cros_gralloc_handle* crosHandle = (cros_gralloc_handle*)handle;
	if (!crosHandle) {
		wlr_log(WLR_ERROR, "Not cros gralloc");
	} else {
		log_cros_gralloc_handle(crosHandle);
		cros_gralloc_to_wlr_dmabuf(crosHandle, dmabuf);
	}


	dmabuf->fd[0] = gralloc_handle(handle)->prime_fd;

	if (dmabuf->fd[0] < 0) {
		wlr_log(WLR_ERROR, "Failed to get dmabuf from AHardwareBuffer");
		ahb_destroy(&buffer->base);
		return NULL;
	}

	wlr_log(WLR_DEBUG, "wlr_dmabuf_attributes:");
	wlr_log(WLR_DEBUG, "  %dx%d, format: 0x%x, modifier: 0x%lx",
			dmabuf->width, dmabuf->height, dmabuf->format, dmabuf->modifier);
	for (int i = 0; i < dmabuf->n_planes; ++i) {
		wlr_log(WLR_DEBUG, "  Plane %d: fd=%d, stride=%u, offset=%u",
				i, dmabuf->fd[i], dmabuf->stride[i], dmabuf->offset[i]);
	}


    wlr_dmabuf_attributes_copy(&buffer->dmabuf, dmabuf);

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

extern struct wlr_swapchain *wlr_ahb_swapchain_create_for_output(struct wlr_output *output) {
	struct wlr_buffer *buffer = wlr_allocator_create_buffer(output->allocator, output->width, output->height, &output->swapchain->format);
	struct wlr_dmabuf_attributes attribs = {0};
	wlr_buffer_get_dmabuf(buffer, &attribs);
	// wlr_buffer_drop(buffer);
	struct wlr_swapchain *ahb_swapchain = wlr_ahb_swapchain_create_with_dmabuf_attribs(&attribs);
	wlr_buffer_drop(buffer);
	ahb_swapchain->format = output->swapchain->format;
	wlr_drm_format_copy(&ahb_swapchain->format, &output->swapchain->format);
	memcpy(&ahb_swapchain->format, &output->swapchain->format, sizeof(ahb_swapchain->format));
	return ahb_swapchain;
}

struct wlr_swapchain *wlr_ahb_swapchain_create_with_dmabuf_attribs(struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_swapchain *swapchain = (struct wlr_swapchain *)calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->allocator = NULL;
	swapchain->width = dmabuf->width;
	swapchain->height = dmabuf->height;


	struct wlr_swapchain_slot *slot = &swapchain->slots[0];
	slot->acquired = false;
	slot->age = 0;
	slot->buffer = &create_wlr_ahb_buffer(dmabuf)->base;

	if (slot->buffer == NULL) {
		wlr_log(WLR_ERROR, "Failed to create AHB swapchain");
		free(swapchain);
		return NULL;
	}

	wlr_log(WLR_DEBUG, "AHB swapchain created");
	return swapchain;
}

