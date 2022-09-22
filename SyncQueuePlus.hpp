#pragma once

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class SyncQueuePlus {
public:
  SyncQueuePlus(size_t maxSize) : m_maxSize(maxSize), m_needStop(false) {
    // std::cout << "sync queue plus is create, tid: " << std::this_thread::get_id() << std::endl;
    printf("sync queue plus is create, tid: %u \n", std::this_thread::get_id());
  }

  void Put(const T &x) {
    // std::cout << "put &x, tid: " << std::this_thread::get_id() << std::endl;
    printf("put &x, tid: %u \n", std::this_thread::get_id());
    Add(x);
  }

  void Put(T &&x) {
    // std::cout << "put &&x, tid: " << std::this_thread::get_id() << std::endl;
    printf("put &&x, tid: %u \n", std::this_thread::get_id());
    Add(std::forward<T>(x));
  }

  void Take(T &x) {
    std::unique_lock<std::mutex> locker(m_mutex);
    m_notEmpty.wait(locker, [this]{ return m_needStop || !IsEmpty(); });
    if (m_needStop) {
      // std::cout << "task &x is stop, tid: " << std::this_thread::get_id() << std::endl;
      printf("task &x is stop, tid: %u \n", std::this_thread::get_id());
      return;
    }
    // std::cout << "take &x is run, tid: " << std::this_thread::get_id() << std::endl;
    printf("take &x is run, tid: %u \n", std::this_thread::get_id());
    x = m_queue.front();
    m_queue.pop_front();
    m_notFull.notify_one();
  }

  void Take(std::list<T> &xList) {
    std::unique_lock<std::mutex> locker(m_mutex);
    m_notEmpty.wait(locker, [this]{ return m_needStop || !IsEmpty(); });
    if (m_needStop) {
      // std::cout << "task &xList is stop, tid: " << std::this_thread::get_id() << std::endl;
      printf("task &xList is stop, tid: %u \n", std::this_thread::get_id());
      return;
    }
    // std::cout << "take &xList is run, tid: " << std::this_thread::get_id() << std::endl;
    printf("take &xList is run, tid: %u \n", std::this_thread::get_id());
    xList = std::move(m_queue);
    m_notFull.notify_one();
  }

  void Stop() {
    {
      std::lock_guard<std::mutex> locker(m_mutex);
      m_needStop = true;
    }
    // std::cout << "stop is run, tid: " << std::this_thread::get_id() << std::endl;
    printf("stop is run, tid: %u \n", std::this_thread::get_id());
    m_notFull.notify_all();
    m_notEmpty.notify_all();
  }

private:
  bool IsFull() const {
    // std::cout << "buffer is full, waiting..." << " tid: " << std::this_thread::get_id() << std::endl;
    printf("buffer is full, waiting... tid: %u \n", std::this_thread::get_id());
    return m_queue.size() >= m_maxSize;
  }

  bool IsEmpty() const {
    // std::cout << "buffer is empty, waiting..." << " tid: " << std::this_thread::get_id() << std::endl;
    printf("buffer is empty, waiting... tid: %u \n", std::this_thread::get_id());
    return m_queue.empty();
  }

  template<typename F>
  void Add(F &&x) {
    std::unique_lock<std::mutex> locker(m_mutex);
    m_notFull.wait(locker, [this]{ return m_needStop || !IsFull(); });
    if (m_needStop) {
      // std::cout << "add &&x is stop, tid: " << std::this_thread::get_id() << std::endl;
      printf("add &&x is stop, tid: %u \n", std::this_thread::get_id());
      return;
    }
    // std::cout << "add &&x is run, tid: " << std::this_thread::get_id() << std::endl;
    printf("add &&x is run, tid: %u \n", std::this_thread::get_id());
    m_queue.push_back(std::forward<F>(x));
    m_notEmpty.notify_one();
  }

private:
  std::list<T> m_queue;
  std::mutex m_mutex;
  size_t m_maxSize;
  std::condition_variable m_notEmpty;
  std::condition_variable m_notFull;
  bool m_needStop;
};
