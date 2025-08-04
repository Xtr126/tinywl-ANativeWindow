#include <android/hardware_buffer.h>
#include <string.h>
#include <unistd.h>
#include "cutils/native_handle.h"
#include "drm_fourcc.h"
#include "vndk/hardware_buffer.h"
#include "buffer_utils.h"
#include <fcntl.h>
#include "cros_gralloc_handle.h"
#include "cros_gralloc_util.h"

extern "C" {
    #include <wlr/interfaces/wlr_buffer.h>
    #include <wlr/util/log.h>
}

// Convert Android format to DRM format
uint32_t android_to_drm_format(uint32_t android_format) {
    switch(android_format) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: 
            return DRM_FORMAT_ABGR8888;

        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM: 
            return DRM_FORMAT_XBGR8888;

        // Add other formats as needed
        default:
            return DRM_FORMAT_INVALID;
    }
}


// Get DMA-BUF attributes from AHardwareBuffer
bool AHardwareBuffer_getDmabufAttributes(AHardwareBuffer *ahb, 
                           struct wlr_dmabuf_attributes *outAtrribs) {
    const buffer_handle_t handle = AHardwareBuffer_getNativeHandle(ahb);
	
	if (!handle) {
		wlr_log(WLR_ERROR, "Failed to get handle from AHardwareBuffer");
		return false;
	}
    cros_gralloc_handle* crosHandle = (cros_gralloc_handle*)handle;
	
	log_cros_gralloc_handle(crosHandle);
	struct wlr_dmabuf_attributes attribs = cros_gralloc_to_wlr_dmabuf(crosHandle);
	// dmabuf->fd[0] = gralloc_handle(handle)->prime_fd;

	if (attribs.fd[0] < 0) {
		wlr_log(WLR_ERROR, "Failed to get dmabuf fd from AHardwareBuffer");
		return false;
	}

	wlr_log(WLR_DEBUG, "wlr_dmabuf_attributes:");
	wlr_log(WLR_DEBUG, "  %dx%d, format: 0x%x, modifier: 0x%lx",
			attribs.width, attribs.height, attribs.format, attribs.modifier);
	
            for (int i = 0; i < attribs.n_planes; ++i)
		wlr_log(WLR_DEBUG, "  Plane %d: fd=%d, stride=%u, offset=%u",
				i, attribs.fd[i], attribs.stride[i], attribs.offset[i]);
                

    wlr_dmabuf_attributes_copy(outAtrribs, &attribs);
    return true;

}

void ANativeWindow_sendWlrBuffer(struct wlr_buffer *wlr_buffer, BufferManager *buffer_presenter) {
    struct wlr_dmabuf_attributes dmabuf_attrs = {0};

    if (!wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf_attrs)) {
		wlr_log(WLR_ERROR, "wlr_buffer_get_dmabuf failed");
		return;
    } 
        
    // Create AHardwareBuffer from DMA-BUF
    AHardwareBuffer_Desc ahb_desc = {
        .width = static_cast<uint32_t>(wlr_buffer->width),
        .height = static_cast<uint32_t>(wlr_buffer->height),
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };

    native_handle_t *handle = native_handle_create(1, 4);
    handle->data[0] = fcntl(dmabuf_attrs.fd[0], F_DUPFD_CLOEXEC, 0); // DMA-BUF FD
    handle->data[1] = dmabuf_attrs.stride[0];               // Buffer stride
    handle->data[2] = 0;                                     // Offset
    handle->data[3] = (uint32_t)dmabuf_attrs.modifier;   // Modifier low
    handle->data[4] = dmabuf_attrs.modifier >> 32;       // Modifier high

    AHardwareBuffer *ahb;
    int ret = AHardwareBuffer_createFromHandle(
        &ahb_desc, handle, AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER, &ahb
    );

    if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to create AHardwareBuffer from handle: %s (%d)", strerror(ret), ret);
        return;
    }

    buffer_presenter_send_buffer(buffer_presenter, ahb, -1, NULL, NULL);
}