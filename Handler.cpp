#include "Handler.h"
#include <android/looper.h>
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <queue>

struct Handler::Impl {
    Impl(ALooper* looper) : mLooper(looper) {
        if (pipe2(pipeFd, O_NONBLOCK) != 0) {
            throw std::runtime_error("Pipe creation failed");
        }

        int result = ALooper_addFd(
            mLooper,
            pipeFd[0],
            ALOOPER_POLL_CALLBACK,
            ALOOPER_EVENT_INPUT,
            &Impl::LooperCallback,
            this
        );

        if (result != 1) {
            close(pipeFd[0]);
            close(pipeFd[1]);
            throw std::runtime_error("Failed to attach fd to looper");
        }
    }

    ~Impl() {
        ALooper_removeFd(mLooper, pipeFd[0]);
        close(pipeFd[0]);
        close(pipeFd[1]);
    }

    bool post(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            taskQueue.push(std::move(task));
        }

        uint8_t wakeByte = 1;
        return write(pipeFd[1], &wakeByte, sizeof(wakeByte)) > 0;
    }

private:
    static int LooperCallback(int fd, int events, void* data) {
        if (events & ALOOPER_EVENT_INPUT) {
            reinterpret_cast<Impl*>(data)->drainAndExecute();
        }
        return 1;
    }

    void drainAndExecute() {
        uint8_t buffer[256];
        while (read(pipeFd[0], buffer, sizeof(buffer)) > 0) {}

        std::queue<std::function<void()>> tasks;
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.swap(taskQueue);
        }

        while (!tasks.empty()) {
            tasks.front()();
            tasks.pop();
        }
    }

    ALooper* mLooper;
    int pipeFd[2];
    std::mutex mutex;
    std::queue<std::function<void()>> taskQueue;
};

Handler::Handler(ALooper* looper) : mImpl(std::make_unique<Impl>(looper)) {}
Handler::~Handler() = default;

bool Handler::post(std::function<void()> task) {
    return mImpl->post(std::move(task));
}