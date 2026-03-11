#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <cstdint>
#include <future>
#include <utility>
#include <type_traits>
#include <tuple>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <queue>
#include <unordered_map>

class EventUpdateHandler {
public:
    static EventUpdateHandler& GetInstance() {
        static EventUpdateHandler instance;
        return instance;
    }

    // 原有接口
    void AddEvent(const std::function<void()>& func, bool once = true);
    bool RemoveEvent(uintptr_t eventId);
    void ClearEvents();
    size_t GetEventCount() const;
    void Update();
    void Shutdown();

    // 新增：带ID返回的AddEvent
    uintptr_t AddEventWithId(const std::function<void()>& func, bool once = true);

    // 修改后的 AddEventWithResult，自动处理死锁
    template<typename Func, typename... Args>
    auto AddEventWithResult(Func&& func, Args&&... args)
    -> std::future<decltype(func(std::forward<Args>(args)...))>
    {
        using ReturnType = decltype(func(std::forward<Args>(args)...));

        // 检查是否在 Update 线程中
        if (IsInUpdateThread()) {
            // 如果在 Update 线程中，直接执行并返回结果
            std::promise<ReturnType> promise;

            try {
                if constexpr (std::is_void_v<ReturnType>) {
                    func(std::forward<Args>(args)...);
                    promise.set_value();
                } else {
                    ReturnType result = func(std::forward<Args>(args)...);
                    promise.set_value(std::move(result));
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }

            return promise.get_future();
        }

        // 否则使用原来的异步方式
        auto promise = std::make_shared<std::promise<ReturnType>>();
        std::future<ReturnType> future = promise->get_future();

        auto wrapper = [promise, func = std::forward<Func>(func),
                args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            try {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::apply(func, args_tuple);
                    promise->set_value();
                } else {
                    auto result = std::apply(func, args_tuple);
                    promise->set_value(std::move(result));
                }
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };

        AddEvent(wrapper, true);

        return future;
    }

    // 简化调用版本
    template<typename Func, typename... Args>
    auto AddEventR(Func&& func, Args&&... args) {
        return AddEventWithResult(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    // 获取当前是否在 Update 线程中执行
    bool IsInUpdateThread() const {
        std::thread::id currentId = std::this_thread::get_id();
        return currentId == m_updateThreadId.load();
    }

    // 强制立即执行一个事件（用于在 Update 中需要立即执行的情况）
    template<typename Func, typename... Args>
    auto ExecuteImmediately(Func&& func, Args&&... args)
    -> decltype(func(std::forward<Args>(args)...))
    {
        // 如果在 Update 线程中，直接执行
        if (IsInUpdateThread()) {
            return func(std::forward<Args>(args)...);
        }

        // 否则异步执行并等待结果
        auto future = AddEventWithResult(std::forward<Func>(func),
                                         std::forward<Args>(args)...);

        // 等待结果（注意：这可能阻塞当前线程）
        return future.get();
    }

    // 设置是否启用线程安全检查（调试用）
    void EnableThreadSafetyCheck(bool enable) {
        m_enableThreadSafetyCheck = enable;
    }

private:
    EventUpdateHandler() : m_updateThreadId(std::thread::id()), m_shutdown(false), m_enableThreadSafetyCheck(true) {}
    ~EventUpdateHandler() = default;

    EventUpdateHandler(const EventUpdateHandler&) = delete;
    EventUpdateHandler& operator=(const EventUpdateHandler&) = delete;

    class FunctionWrapper {
    public:
        FunctionWrapper(const std::function<void()>& func, bool once, uintptr_t id)
                : m_func(func),
                  m_once(once),
                  m_id(id) {
        }

        // 移动构造函数
        FunctionWrapper(FunctionWrapper&& other) noexcept
                : m_func(std::move(other.m_func)),
                  m_once(other.m_once),
                  m_id(other.m_id) {
        }

        // 移动赋值运算符
        FunctionWrapper& operator=(FunctionWrapper&& other) noexcept {
            if (this != &other) {
                m_func = std::move(other.m_func);
                m_once = other.m_once;
                m_id = other.m_id;
            }
            return *this;
        }

        void operator()() const {
            if (m_func) {
                m_func();
            }
        }

        explicit operator bool() const {
            return static_cast<bool>(m_func);
        }

        bool isOnce() const {
            return m_once;
        }

        uintptr_t getId() const {
            return m_id;
        }

    private:
        std::function<void()> m_func;
        bool m_once;
        uintptr_t m_id;
    };

    uintptr_t GenerateEventId() {
        static std::atomic<uintptr_t> counter(0);
        return ++counter;
    }

    void CheckThreadSafety() {
        if (m_enableThreadSafetyCheck && !IsInUpdateThread()) {
        }
    }

    std::vector<FunctionWrapper> m_events;
    std::queue<FunctionWrapper> m_pendingEvents;
    mutable std::mutex m_mutex;
    std::atomic<std::thread::id> m_updateThreadId;
    std::atomic<bool> m_shutdown;
    std::atomic<bool> m_enableThreadSafetyCheck;
};