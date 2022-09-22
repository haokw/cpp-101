#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class SimpleSyncQueue {
public:
  SimpleSyncQueue(const size_t maxSize) : m_maxSize(maxSize) {}

  bool Put(const T &x) {
    std::unique_lock<std::mutex> locker(m_mutex);
    m_notFull.wait(locker, [this]{ return !IsFull(); });
    m_queue.push_back(x);
    m_notEmpty.notify_one();
    return true;
  }

  bool Take(T &x) {
    std::unique_lock<std::mutex> locker(m_mutex);
    m_notEmpty.wait(locker, [this]{ return !IsEmpty(); });
    x = m_queue.front();
    m_queue.pop_front();
    m_notFull.notify_one();
    return true;
  }

private:
  bool IsEmpty() const {
    return m_queue.empty();
  }

  bool IsFull() const {
    return m_queue.size() >= m_maxSize;
  }

private:
  std::list<T> m_queue;
  size_t m_maxSize;
  std::mutex m_mutex;
  std::condition_variable m_notEmpty;
  std::condition_variable m_notFull;
};
