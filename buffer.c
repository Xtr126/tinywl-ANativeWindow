#include <string.h>
#include <unistd.h>
#include "cutils/native_handle.h"
#include "drm_fourcc.h"
#include "nativebase/nativebase.h"
#include "vndk/window.h"
#include "vndk/hardware_buffer.h"
#include "buffer.h"
#include "dmabuf.h"
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/util/log.h>


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

void ANativeWindow_render_scene(ANativeWindow *window, struct wlr_output *output, 
        struct wlr_scene_output *scene_output) {
    // ANativeWindow_setBuffersGeometry(window, output->width, output->height,AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);

    wlr_log(WLR_DEBUG, "Dequeuing buffer");
    ANativeWindowBuffer *anb;
    int fence_fd = -1;
    int ret = ANativeWindow_dequeueBuffer(window, &anb, &fence_fd);
    if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to dequeue buffer: %s (%d)", strerror(-ret), -ret);
        return;
    }
    
    if (fence_fd >= 0) {
        wlr_log(WLR_DEBUG, "Waiting for buffer fence");
        if (sync_wait(fence_fd, -1)) {
            wlr_log(WLR_ERROR, "Failed to wait for fence");
            close(fence_fd);
            return;
        }
        close(fence_fd);
    }
    
    wlr_log(WLR_DEBUG, "Getting hardware buffer");
    AHardwareBuffer *ahb = ANativeWindowBuffer_getHardwareBuffer(anb);
    if (!ahb) {
        wlr_log(WLR_ERROR, "Failed to get hardware buffer");
        return;
    }
    
    wlr_log(WLR_DEBUG, "Creating DMA-BUF attributes");
    struct wlr_dmabuf_attributes attribs;
    if (!AHardwareBuffer_getDmabufAttributes(ahb, &attribs)) {
        wlr_log(WLR_ERROR, "Failed to get DMA-BUF attributes");
        return;
    }
    
    wlr_log(WLR_DEBUG, "Creating wlroots buffer");
    struct wlr_dmabuf_buffer *dmabuf_buffer = dmabuf_buffer_create(&attribs);
    if (!dmabuf_buffer) {
        wlr_log(WLR_ERROR, "Failed to create dmabuf buffer");
        close(attribs.fd[0]);
        return;
    }

    struct wlr_buffer *buffer = &dmabuf_buffer->base; 
    
    wlr_log(WLR_DEBUG, "Attaching buffer to output");
    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_buffer(&state, buffer);
    if (!wlr_output_commit_state(output, &state)) {
        wlr_log(WLR_ERROR, "Failed to commit output state");
        wlr_output_state_finish(&state);
        wlr_buffer_drop(buffer);
        return;
    }
    wlr_output_state_finish(&state);

    wlr_log(WLR_DEBUG, "Rendering scene");
    if (!wlr_scene_output_commit(scene_output, NULL)) {
        wlr_log(WLR_ERROR, "Failed to commit scene output");
        wlr_buffer_drop(buffer);
        return;
    }
    
    wlr_log(WLR_DEBUG, "Releasing wlroots buffer");
    wlr_buffer_drop(buffer);
    
    wlr_log(WLR_DEBUG, "Queueing buffer back to Android");
    if (ANativeWindow_queueBuffer(window, anb, -1)) {
        wlr_log(WLR_ERROR, "Failed to queue buffer");
    }
}