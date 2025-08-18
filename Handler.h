#ifndef HANDLER_H
#define HANDLER_H

#include <android/looper.h>
#include <functional>
#include <memory>

/**
 * C++ implementation mimicking Android's Handler.post() functionality
 * using ALooper and pipe-based event notification.
 */
class Handler {
public:
    /**
     * Constructs a Handler attached to the specified ALooper
     * 
     * @param looper The ALooper to attach to (must not be null)
     * @throws std::runtime_error if pipe creation or ALooper attachment fails
     */
    explicit Handler(ALooper* looper);
    
    /**
     * Destructor - cleans up pipe and ALooper registration
     */
    ~Handler();

    /**
     * Posts a task to be executed on the Handler's looper thread
     * 
     * @param task Callable to be executed
     * @return true if task was successfully queued, false on critical failure
     */
    bool post(std::function<void()> task);

    // Delete copy/move constructors and assignment operators
    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler(Handler&&) = delete;
    Handler& operator=(Handler&&) = delete;

private:
    // Forward declarations for private implementation
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

#endif // HANDLER_H