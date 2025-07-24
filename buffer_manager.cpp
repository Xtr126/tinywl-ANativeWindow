#include "buffer_manager.h"

#include <android/surface_control.h>
#include <android/log.h>
#include <cstdlib>
#include <new>
#include <unistd.h> // For close()

extern "C" {
    #include <wlr/util/log.h>
} 
#define ALOGE(...) wlr_log(WLR_ERROR, __VA_ARGS__)
#define ALOGI(...) wlr_log(WLR_DEBUG, __VA_ARGS__)

/**
 * @brief The internal state of the BufferManager.
 * This struct now holds the ASurfaceControl, which is the target for transactions.
 */
struct BufferManager {
    // We keep the window reference for completeness and potential debugging.
    ANativeWindow* window;
};

// C-style function implementations callable from external code.

BufferManager* buffer_manager_create(ANativeWindow* window) {
    if (!window) {
        ALOGE("Cannot create BufferManager with a null ANativeWindow.");
        return nullptr;
    }

    // Create the SurfaceControl from the window. This is the object
    // that transactions will operate on.
    ASurfaceControl* surface_control = ASurfaceControl_createFromWindow(window, "BufferManagerSurfaceControl");
    if (!surface_control) {
        ALOGE("Failed to create ASurfaceControl from window.");
        return nullptr;
    }

    // Allocate memory for our state struct.
    BufferManager* manager = new (std::nothrow) BufferManager();
    if (!manager) {
        ALOGE("Failed to allocate memory for BufferManager.");
        // Clean up the surface control if allocation fails.
        ASurfaceControl_release(surface_control);
        return nullptr;
    }

    // Retain a reference to the window. Although the ASurfaceControl also holds
    // a reference, it's good practice for the manager to hold its own.
    ANativeWindow_acquire(window);
    manager->window = window;

    ALOGI("BufferManager created for window %p with SurfaceControl %p", window, surface_control);
    return manager;
}

void buffer_manager_destroy(BufferManager* manager) {
    if (!manager) {
        return;
    }

    if (manager->window) {
        ALOGI("Releasing window %p", manager->window);
        // Release the reference we took in the create function.
        ANativeWindow_release(manager->window);
        manager->window = nullptr;
    }
    
    // Free the memory for the manager struct itself.
    delete manager;
    ALOGI("BufferManager destroyed.");
}

int buffer_manager_send_buffer(BufferManager* manager, AHardwareBuffer* buffer, int acquire_fence_fd) {
    ASurfaceControl* surface_control = ASurfaceControl_createFromWindow(manager->window, "BufferManagerSurfaceControl");
    if (!surface_control) {
        ALOGE("Failed to create ASurfaceControl from window.");
        // If we fail here, we are now responsible for the fence fd.
        if (acquire_fence_fd >= 0) {
            close(acquire_fence_fd);
        }
        return -1;
    }

    if (!buffer) {
        ALOGE("Input AHardwareBuffer is null.");
        if (acquire_fence_fd >= 0) {
            close(acquire_fence_fd);
        }
        return -2;
    }

    // 1. Create a new surface transaction.
    ASurfaceTransaction* transaction = ASurfaceTransaction_create();
    if (!transaction) {
        ALOGE("Failed to create ASurfaceTransaction.");
        if (acquire_fence_fd >= 0) {
            close(acquire_fence_fd);
        }
        return -3;
    }

    // 2. Set the buffer for the target surface control in the transaction.
    // The transaction takes ownership of the acquire_fence_fd and will close it.
    ASurfaceTransaction_setBuffer(transaction, surface_control, buffer, acquire_fence_fd);

    // 3. Apply the transaction to make the changes visible.
    // This function is void and does not return an error code.
    ASurfaceTransaction_apply(transaction);

    // 4. Delete the transaction object to release its resources.
    ASurfaceTransaction_delete(transaction);

    ALOGI("Releasing SurfaceControl %p", surface_control);
    // Release the SurfaceControl. This is the critical cleanup step.
    ASurfaceControl_release(surface_control);

    // Since apply is void, we assume success if we reached this point
    // without returning an error from earlier checks.
    return 0;
}
