#include "EventUpdateHandler.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>

uintptr_t EventUpdateHandler::AddEventWithId(const std::function<void()>& func, bool once) {
    if (m_shutdown) {
        std::cerr << "EventUpdateHandler: 系统已关闭，无法添加事件" << std::endl;
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    uintptr_t eventId = GenerateEventId();
    m_pendingEvents.emplace(func, once, eventId);
    return eventId;
}

void EventUpdateHandler::AddEvent(const std::function<void()>& func, bool once) {
    AddEventWithId(func, once);
}

bool EventUpdateHandler::RemoveEvent(uintptr_t eventId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (eventId == 0) return false;

    // 从待处理队列中移除
    std::queue<FunctionWrapper> newPendingQueue;
    while (!m_pendingEvents.empty()) {
        auto wrapper = std::move(m_pendingEvents.front());
        m_pendingEvents.pop();
        if (wrapper.getId() != eventId) {
            newPendingQueue.push(std::move(wrapper));
        }
    }
    m_pendingEvents = std::move(newPendingQueue);

    // 从当前事件队列中移除
    auto it = std::remove_if(m_events.begin(), m_events.end(),
                             [eventId](const FunctionWrapper& wrapper) {
                                 return wrapper.getId() == eventId;
                             });

    bool removed = (it != m_events.end());
    m_events.erase(it, m_events.end());

    return removed;
}

void EventUpdateHandler::ClearEvents() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<FunctionWrapper> emptyQueue;
    std::swap(m_pendingEvents, emptyQueue);
    m_events.clear();
}

size_t EventUpdateHandler::GetEventCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events.size() + m_pendingEvents.size();
}

void EventUpdateHandler::Update() {
    // 标记当前线程为 Update 线程
    m_updateThreadId = std::this_thread::get_id();

    // 检查线程安全性（调试用）
    CheckThreadSafety();

    // 第一步：合并待处理事件到临时列表（加锁时间尽量短）
    std::vector<FunctionWrapper> eventsToExecute;
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 如果系统已关闭，清空所有事件
        if (m_shutdown) {
            m_events.clear();
            std::queue<FunctionWrapper> emptyQueue;
            std::swap(m_pendingEvents, emptyQueue);
            return;
        }

        // 预分配空间以提高性能
        size_t totalEvents = m_events.size() + m_pendingEvents.size();
        eventsToExecute.reserve(totalEvents);

        // 将当前事件移到临时列表
        for (auto& event : m_events) {
            eventsToExecute.push_back(std::move(event));
        }
        m_events.clear();

        // 将待处理事件移到临时列表
        while (!m_pendingEvents.empty()) {
            eventsToExecute.push_back(std::move(m_pendingEvents.front()));
            m_pendingEvents.pop();
        }
    }

    // 第二步：执行事件并收集重复性事件
    std::vector<FunctionWrapper> recurringEvents;
    recurringEvents.reserve(eventsToExecute.size());

    for (auto& event : eventsToExecute) {
        if (!event || m_shutdown) continue;

        try {
            event();  // 执行事件

            // 如果是重复性事件，保存起来
            if (!event.isOnce()) {
                recurringEvents.push_back(std::move(event));
            }
            // 一次性事件执行后自动丢弃
        } catch (const std::exception& e) {
            std::cerr << "EventUpdateHandler: 事件执行异常: " << e.what() << std::endl;
            // 即使是异常，也保留重复性事件
            if (!event.isOnce() && static_cast<bool>(event)) {
                recurringEvents.push_back(std::move(event));
            }
        } catch (...) {
            std::cerr << "EventUpdateHandler: 未知异常" << std::endl;
            // 即使是异常，也保留重复性事件
            if (!event.isOnce() && static_cast<bool>(event)) {
                recurringEvents.push_back(std::move(event));
            }
        }
    }

    // 第三步：将重复性事件放回队列
    if (!recurringEvents.empty() && !m_shutdown) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.reserve(m_events.size() + recurringEvents.size());
        for (auto& event : recurringEvents) {
            if (static_cast<bool>(event)) {
                m_events.push_back(std::move(event));
            }
        }
    }

    // 清除 Update 线程标记
    m_updateThreadId = std::thread::id();
}

void EventUpdateHandler::Shutdown() {
    m_shutdown = true;
    ClearEvents();
    // 等待所有事件执行完成
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}