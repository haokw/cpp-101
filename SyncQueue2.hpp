#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

// error
// m_size -> m_maxSize
// Put(T &x) -> Put(const T &x)
// which (IsFull()) -> while (IsFull())
// m_empty -> m_notEmpty()
// std::lock_guard<mutex> -> std::lock_guard<std::mutex>
// bool IsEmpty() -> bool IsEmpty() const
// std::condition_variable -> std::condition_variable_any
// m_queue.pop_back(); -> m_queue.pop_front();

template<typename T>
class SyncQueue {
public:
  SyncQueue(size_t maxSize) : m_maxSize(maxSize) {}

  bool Put(const T &x) {
    std::lock_guard<std::mutex> locker(m_mutex);
    while (IsFull()) {
      m_notFull.wait(m_mutex);
    }
    m_queue.push_back(x);
    m_notEmpty.notify_one();

    return true;
  }

  bool Take(T &x) {
    std::lock_guard<std::mutex> locker(m_mutex);
    while (IsEmpty()) {
      m_notEmpty.wait(m_mutex);
    }
    x = m_queue.front();
    m_queue.pop_front();
    m_notFull.notify_one();

    return true;
  }

  size_t Size() {
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_queue.size();
  }

private:
  bool IsEmpty() const {
    return m_queue.empty();
  }

  bool IsFull() const {
    return m_queue.size() >= m_maxSize;
  }

private:
  std::list<T> m_queue; // task buffer
  size_t m_maxSize;
  std::mutex m_mutex; // mutex for condition
  std::condition_variable_any m_notFull;
  std::condition_variable_any m_notEmpty;
};
