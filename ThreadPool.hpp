#pragma once

#include <iostream>
#include <list>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>

#include "SyncQueuePlus.hpp"

const int MaxTaskCount = 100;

class ThreadPool {
public:
  using Task = std::function<void()>;

  ThreadPool(int numThreads = std::thread::hardware_concurrency()) : m_queue(MaxTaskCount) {
    // std::cout << "hardware_concurrency: " << numThreads << std::endl;
    printf("hardware_concurrency: %d \n", numThreads);
    Start(numThreads);
  }

  ~ThreadPool(void) {
    Stop();
  }

  void AddTask(Task &&task) {
    m_queue.Put(std::forward<Task>(task));
  }

  void AddTask(const Task &task) {
    m_queue.Put(task);
  }

  void Stop() {
    std::call_once(m_flag, [this] { StopThreadGroup(); });
  }

private:
  void Start(int numThreads) {
    m_running = true;
    for (int i = 0; i < numThreads; ++i) {
      m_threadGroup.push_back(
          std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
    }
  }

  void RunListInThread() {
    while (m_running) {
      std::list<Task> list;
      m_queue.Take(list);
      for (auto &task : list) {
        if (!m_running) {
          return;
        }
        task();
      }
    }
  }

  void RunInThread() {
    while (m_running) {
      Task task;
      m_queue.Take(task);
      if (!m_running) {
        return;
      }
      task();
    }
  }

  void StopThreadGroup() {
    m_queue.Stop();
    m_running = false;
    for (auto thread : m_threadGroup) {
      if (thread) {
        thread->join();
      }
      m_threadGroup.clear();
    }
  }

private:
  std::list<std::shared_ptr<std::thread>> m_threadGroup;
  SyncQueuePlus<Task> m_queue;
  std::atomic_bool m_running;
  std::once_flag m_flag;
};
