#pragma once

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <utility>

class NetLoop {
public:
    using Handler = std::function<void(int fd, uint32_t events)>;

    NetLoop() = default;
    NetLoop(const NetLoop&) = delete;
    NetLoop& operator=(const NetLoop&) = delete;

    ~NetLoop() {
        stop();
        if (wakefd_ >= 0) {
            close(wakefd_);
            wakefd_ = -1;
        }
        if (epfd_ >= 0) {
            close(epfd_);
            epfd_ = -1;
        }
    }

    int init() {
        epfd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epfd_ < 0) {
            return -1;
        }

        wakefd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (wakefd_ < 0) {
            close(epfd_);
            epfd_ = -1;
            return -2;
        }

        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = wakefd_;
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, wakefd_, &ev) < 0) {
            int saved = errno;
            close(wakefd_);
            wakefd_ = -1;
            close(epfd_);
            epfd_ = -1;
            errno = saved;
            return -3;
        }

        running_.store(true, std::memory_order_release);
        return 0;
    }

    /*
        The following are all wrapper functions for epoll system calls.
    */

    int add(int fd, uint32_t events, Handler h) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;

        {
            std::lock_guard<std::mutex> lock(hMutex_);
            handlers_[fd] = std::move(h);
        }

        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
            int saved = errno;
            {
                std::lock_guard<std::mutex> lock(hMutex_);
                handlers_.erase(fd);
            }
            errno = saved;
            return -1;
        }
        return 0;
    }

    int mod(int fd, uint32_t events) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0 ? -1 : 0;
    }

    int del(int fd) {
        {
            std::lock_guard<std::mutex> lock(hMutex_);
            handlers_.erase(fd);
        }
        return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0 ? -1 : 0;
    }

    int run() {
        while (running_.load(std::memory_order_acquire)) {
            epoll_event evs[64];
            int n = epoll_wait(epfd_, evs, 64, -1);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return -1;
            }

            for (int i = 0; i < n; ++i) {
                int fd = evs[i].data.fd;
                if (fd == wakefd_) {
                    uint64_t value = 0;
                    (void)read(wakefd_, &value, sizeof(value));
                    continue;
                }

                Handler h;
                {
                    std::lock_guard<std::mutex> lock(hMutex_);
                    auto it = handlers_.find(fd);
                    if (it == handlers_.end()) {
                        continue;
                    }
                    h = it->second;
                }
                h(fd, evs[i].events);
            }
        }
        return 0;
    }

    void stop() {
        running_.store(false, std::memory_order_release);
        if (wakefd_ >= 0) {
            uint64_t one = 1;
            (void)write(wakefd_, &one, sizeof(one));
        }
    }

private:
    int epfd_ = -1;
    int wakefd_ = -1;
    std::atomic<bool> running_{false};
    std::mutex hMutex_;
    std::unordered_map<int, Handler> handlers_;
};