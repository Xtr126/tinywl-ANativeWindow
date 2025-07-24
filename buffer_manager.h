#pragma once

#include <android/native_window.h>
#include <android/hardware_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An opaque struct holding the state for the buffer manager.
 * The internal implementation is hidden from the C interface.
 */
typedef struct BufferManager BufferManager;

/**
 * @brief Creates a new BufferManager instance.
 *
 * This function creates an ASurfaceControl from the provided ANativeWindow
 * and initializes the necessary components to manage transactions.
 *
 * @param window The ANativeWindow to which buffers will be sent. The caller
 * retains ownership of the window and must ensure its lifetime
 * exceeds that of the BufferManager.
 * @return A pointer to the newly created BufferManager, or NULL on failure.
 */
BufferManager* buffer_manager_create(ANativeWindow* window);

/**
 * @brief Destroys a BufferManager instance and cleans up its resources.
 *
 * This function releases the underlying ASurfaceControl.
 *
 * @param manager A pointer to the BufferManager instance to be destroyed.
 * If manager is NULL, the function does nothing.
 */
void buffer_manager_destroy(BufferManager* manager);

/**
 * @brief Sends a hardware buffer to the associated ANativeWindow using ASurfaceTransaction.
 *
 * This function creates and applies a transaction to set the provided buffer
 * on the window's ASurfaceControl.
 *
 * @param manager The BufferManager instance.
 * @param buffer The AHardwareBuffer to display. The buffer is not acquired by this
 * function; the caller is responsible for managing its lifecycle.
 * @param acquire_fence_fd A file descriptor for a sync fence that will signal
 * when the buffer is ready to be read. The transaction will wait
 * on this fence. The function takes ownership of the fd and will
 * close it. Use -1 if no fence is needed.
 * @return 0 on success, or a negative value on error.
 */
int buffer_manager_send_buffer(BufferManager* manager, AHardwareBuffer* buffer, int acquire_fence_fd);


#ifdef __cplusplus
}
#endif
