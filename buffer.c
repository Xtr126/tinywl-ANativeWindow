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

void ANativeWindow_render_scene(ANativeWindow *window, struct wlr_output *output, struct wlr_scene_output * scene_output) {
        ANativeWindow_setBuffersGeometry(window, output->width, output->height,
        AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);

        // Dequeue buffer
        ANativeWindowBuffer *anb;
        int fence_fd = -1;
        ANativeWindow_dequeueBuffer(window, &anb, &fence_fd);
        
        // Wait for buffer availability
        if (fence_fd >= 0) {
            sync_wait(fence_fd, -1);
            close(fence_fd);
        }
        
        // Get hardware buffer
        AHardwareBuffer *ahb = ANativeWindowBuffer_getHardwareBuffer(anb);
        
        // Create DMA-BUF attributes
        struct wlr_dmabuf_attributes attribs;
        if (!AHardwareBuffer_getDmabufAttributes(ahb, &attribs)) {
            // Handle error
            return;
        }
        
        // Create wlroots buffer
        struct wlr_dmabuf_buffer *dmabuf_buffer = dmabuf_buffer_create(&attribs);
        if (!dmabuf_buffer) {
            close(attribs.fd[0]);  // Cleanup on failure
            return;;
        }

        struct wlr_buffer *buffer = &dmabuf_buffer->base; 
        
        // Attach buffer
        struct wlr_output_state state;
	    wlr_output_state_init(&state);
        wlr_output_state_set_buffer(&state, buffer);
        wlr_output_commit_state(output, &state);
	    wlr_output_state_finish(&state);

        // Render buffer
        /* Render the scene if needed and commit the output */
        wlr_scene_output_commit(scene_output, NULL);

                
        // Release wlroots buffer (doesn't close original FD)
        wlr_buffer_drop(buffer);
        
        // Queue buffer back to Android
        ANativeWindow_queueBuffer(window, anb, -1);  // -1 = no release fence
}

