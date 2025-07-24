#include <android/hardware_buffer.h>
#include <string.h>
#include <unistd.h>
#include "buffer_manager.h"
#include "cutils/native_handle.h"
#include "drm_fourcc.h"
#include "vndk/hardware_buffer.h"
#include "buffer.h"
#include "dmabuf.h"
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/util/log.h>
#include <fcntl.h>

// Convert Android format to DRM format
uint32_t android_to_drm_format(uint32_t android_format) {
    switch(android_format) {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: 
        return DRM_FORMAT_ABGR8888;
    // Add other formats as needed
    default:
        return DRM_FORMAT_INVALID;
    }
}


// Get DMA-BUF attributes from AHardwareBuffer
bool AHardwareBuffer_getDmabufAttributes(AHardwareBuffer *ahb, 
                           struct wlr_dmabuf_attributes *attribs) {
    AHardwareBuffer_Desc desc;
    AHardwareBuffer_describe(ahb, &desc);
    
    // Get native handle (requires Android 8.0+)
    const native_handle_t *handle = AHardwareBuffer_getNativeHandle(ahb);
    if (!handle || handle->numFds < 1) return false;
    
    memset(attribs, 0, sizeof(*attribs));
    attribs->n_planes = 1;
    attribs->width = desc.width;
    attribs->height = desc.height;
    attribs->format = android_to_drm_format(desc.format);
    
    // Get modifier 
    attribs->modifier = DRM_FORMAT_MOD_INVALID;
    
    // Get first plane's FD
    attribs->fd[0] = dup(handle->data[0]);  // Duplicate FD for ownership
    attribs->stride[0] = desc.stride;
    attribs->offset[0] = 0;
    
    return true;
}

void ANativeWindow_sendWlrBuffer(struct wlr_buffer *wlr_buffer, BufferManager *buffer_manager) {

    struct wlr_dmabuf_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    // Create AHardwareBuffer from DMA-BUF
    AHardwareBuffer_Desc ahb_desc = {
        .width = wlr_buffer->width,
        .height = wlr_buffer->height,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };


    native_handle_t *handle = native_handle_create(1, 4);
    handle->data[0] = fcntl(buffer->dmabuf.fd[0], F_DUPFD_CLOEXEC, 0); // DMA-BUF FD
    handle->data[1] = buffer->dmabuf.stride[0];               // Buffer stride
    handle->data[2] = 0;                                     // Offset
    handle->data[3] = (uint32_t)buffer->dmabuf.modifier;   // Modifier low
    handle->data[4] = buffer->dmabuf.modifier >> 32;       // Modifier high

    AHardwareBuffer *ahb;
    int ret = AHardwareBuffer_createFromHandle(
        &ahb_desc, handle, AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER, &ahb
    );

    if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to create AHardwareBuffer from handle: %s (%d)", strerror(ret), ret);
        return;
    }

    buffer_manager_send_buffer(buffer_manager, ahb, -1);
}