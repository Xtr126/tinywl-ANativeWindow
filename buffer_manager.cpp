#include "buffer_manager.h"

#include <android/api-level.h>
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
 */
struct BufferManager {
    // We keep the window reference for completeness and potential debugging.
    ANativeWindow* window;
};

// --- Start of API-specific implementation details ---

// This struct is used on older APIs to pass the user's callback
// and context through the ASurfaceTransaction_setOnComplete mechanism.
struct OnCompleteContext {
    BufferManager_OnReleaseCallback user_callback;
    void* user_context;
};

/**
 * @brief Adapter callback for APIs < 36.
 *
 * This function is registered with ASurfaceTransaction_setOnComplete. It extracts
 * the release fence for the *previous* buffer from the transaction stats
 * and forwards it to the user's final callback.
 */
static void on_transaction_complete_legacy_adapter(void* context, ASurfaceTransactionStats* stats) {
    if (!context || !stats) {
        if (context) delete static_cast<OnCompleteContext*>(context);
        ALOGE("Legacy adapter callback received null context or stats.");
        return;
    }

    OnCompleteContext* wrapped_context = static_cast<OnCompleteContext*>(context);


    if (wrapped_context->user_callback) {
        wrapped_context->user_callback(wrapped_context->user_context, -1 /* Already released */);
    } 

    // Clean up the context wrapper we allocated.
    delete wrapped_context;
}

// --- End of API-specific implementation details ---


BufferManager* buffer_manager_create(ANativeWindow* window) {
    if (!window) {
        ALOGE("Cannot create BufferManager with a null ANativeWindow.");
        return nullptr;
    }

    ASurfaceControl* surface_control = ASurfaceControl_createFromWindow(window, "BufferManagerSurfaceControl");
    if (!surface_control) {
        ALOGE("Failed to create ASurfaceControl from window.");
        return nullptr;
    }

    BufferManager* manager = new (std::nothrow) BufferManager();
    if (!manager) {
        ALOGE("Failed to allocate memory for BufferManager.");
        ASurfaceControl_release(surface_control);
        return nullptr;
    }

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
        ANativeWindow_release(manager->window);
        manager->window = nullptr;
    }
    
    delete manager;
    ALOGI("BufferManager destroyed.");
}

int buffer_manager_send_buffer(BufferManager* manager,
                               AHardwareBuffer* buffer,
                               int acquire_fence_fd,
                               BufferManager_OnReleaseCallback on_release_callback,
                               void* context) {
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
        if (acquire_fence_fd >= 0) close(acquire_fence_fd);
        return -2;
    }

    ASurfaceTransaction* transaction = ASurfaceTransaction_create();
    if (!transaction) {
        ALOGE("Failed to create ASurfaceTransaction.");
        if (acquire_fence_fd >= 0) close(acquire_fence_fd);
        return -3;
    }

#if __ANDROID_API__ >= 36
    if (on_release_callback) {
        ASurfaceTransaction_setBufferWithRelease(
            transaction, surface_control, buffer, acquire_fence_fd,
            context, on_release_callback);
    } else {
        // If no callback is needed, use the simpler function.
        ASurfaceTransaction_setBuffer(transaction, surface_control, buffer, acquire_fence_fd);
    }
#else
    // For older APIs, we use the setOnComplete callback as an adapter.
    ASurfaceTransaction_setBuffer(transaction, surface_control, buffer, acquire_fence_fd);
    if (on_release_callback) {
        // We must allocate a temporary context to pass our own adapter callback.
        OnCompleteContext* callback_context = new (std::nothrow) OnCompleteContext{
            .user_callback = on_release_callback, 
            .user_context = context,
        };
        if (callback_context) {
            ASurfaceTransaction_setOnComplete(transaction, callback_context, on_transaction_complete_legacy_adapter);
        } else {
            ALOGE("Failed to allocate memory for OnCompleteContext wrapper.");
            // We can't set the callback, but we can still try to apply the transaction.
        }
    }
#endif

    ASurfaceTransaction_apply(transaction);
    ASurfaceTransaction_delete(transaction);

    ALOGI("Releasing SurfaceControl %p", surface_control);
    // Release the SurfaceControl. This is the critical cleanup step.
    ASurfaceControl_release(surface_control);

    // Since apply is void, we assume success if we reached this point
    // without returning an error from earlier checks.
    return 0;
}
