#include <EGL/egl.h>
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

#include <wlr/render/gles2.h>
#include <wlr/render/egl.h>



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

struct wlr_renderer *initialize_egl_and_renderer(ANativeWindow *window) {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
    struct wlr_egl *wlr_egl_instance = NULL;
    struct wlr_renderer *renderer = NULL;

    wlr_log(WLR_INFO, "Initializing EGL for ANativeWindow: %p", (void*)window);

    if (window == NULL) {
        wlr_log(WLR_ERROR, "ANativeWindow pointer is NULL.");
        return NULL;
    }

    // 1. Get EGL Display
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        wlr_log(WLR_ERROR, "Failed to get EGL display: 0x%x", eglGetError());
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL display obtained.");

    // 2. Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        wlr_log(WLR_ERROR, "Failed to initialize EGL: 0x%x", eglGetError());
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL initialized (version %d.%d).", major, minor);

    // 3. Choose EGL Configuration
    EGLConfig config;
    EGLint num_configs;
    const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // Request OpenGL ES 2.0
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,       // Support window surfaces
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,                      // 8-bit alpha for RGBA
        EGL_DEPTH_SIZE, 24,                     // Optional: 24-bit depth buffer
        EGL_NONE
    };

    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs)) {
        wlr_log(WLR_ERROR, "Failed to choose EGL config: 0x%x", eglGetError());
        eglTerminate(display);
        return NULL;
    }
    if (num_configs == 0) {
        wlr_log(WLR_ERROR, "No suitable EGL configurations found.");
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL config chosen.");

    // 4. Create EGL Context
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, // Request OpenGL ES 2.0 context
        EGL_NONE
    };

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        wlr_log(WLR_ERROR, "Failed to create EGL context: 0x%x", eglGetError());
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL context created.");

    // 5. Create EGL Window Surface
    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)window, NULL);
    if (surface == EGL_NO_SURFACE) {
        wlr_log(WLR_ERROR, "Failed to create EGL window surface: 0x%x", eglGetError());
        eglDestroyContext(display, context);
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL window surface created.");

    // 6. Make EGL Context Current
    if (!eglMakeCurrent(display, surface, surface, context)) {
        wlr_log(WLR_ERROR, "Failed to make EGL context current: 0x%x", eglGetError());
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "EGL context made current.");

    // 7. Set ANativeWindow Buffer Geometry
    // This is crucial for ANativeWindow to allocate buffers correctly.
    // Get current dimensions from the window.
    int32_t width = ANativeWindow_getWidth(window);
    int32_t height = ANativeWindow_getHeight(window);

	int ret = ANativeWindow_setBuffersGeometry(window, 0, 0,AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
    if (ret != 0) {
        wlr_log(WLR_ERROR, "Failed to set ANativeWindow buffer geometry: %s (%d)", strerror(-ret), -ret);
        // Clean up EGL resources
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "ANativeWindow buffer geometry set: %dx%d", width, height);


    // 8. Create wlr_egl instance
    wlr_egl_instance = wlr_egl_create_with_context(display, context);
    if (wlr_egl_instance == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_egl instance.");
        // Clean up EGL resources
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);
        return NULL;
    }
    wlr_log(WLR_DEBUG, "wlr_egl instance created.");

    // 9. Create wlr_renderer
    renderer = wlr_gles2_renderer_create(wlr_egl_instance);
    if (renderer == NULL) {
        wlr_log(WLR_ERROR, "Failed to create wlr_gles2_renderer.");
        // Clean up EGL resources (already done by wlr_egl_destroy if it manages them)
        // If wlr_egl_create_with_context takes ownership, no need to destroy EGL manually here.
        // Assuming wlr_egl_destroy handles the underlying EGL resources if it created them.
        return NULL;
    }
    wlr_log(WLR_INFO, "wlr_gles2_renderer created successfully.");

    // Store surface in wlr_egl_instance for later use by the renderer, if needed
    // (wlr_egl might not directly manage the EGLSurface, but the renderer will use it)
    // You might need to pass the surface explicitly to rendering functions later,
    // or associate it with the renderer in a custom way if wlr_renderer doesn't
    // inherently track the EGLSurface it's rendering to.
    // For now, we've made it current, which is enough for the renderer to operate.

    // Note: The EGLSurface 'surface' is now "owned" by the current EGL context.
    // It will be implicitly used by the wlr_renderer.
    // You should *not* destroy 'surface', 'context', or 'display' manually here.
    // They will be managed by the wlr_egl_instance and destroyed when wlr_egl_destroy is called.

    return renderer;
}
