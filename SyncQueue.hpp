#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class SyncQueue {
public:
  SyncQueue(int maxSize): m_maxSize(maxSize) {}

  void Put(const T &x) {
    std::lock_guard<std::mutex> locker(m_mutex);
    while (IsFull()) {
      std::cout << "buffer is full, waiting..." << std::endl;
      m_notFull.wait(m_mutex);
    }
    m_queue.push_back(x);
    m_notEmpty.notify_one();
  }

  void Task(T &x) {
    std::lock_guard<std::mutex> locker(m_mutex);
    while (IsEmpty()) {
      std::cout << "buffer is empty, waiting..." << std::endl;
      m_notEmpty.wait(m_mutex);
    }
    x = m_queue.front();
    m_queue.pop_front();
    m_notFull.notify_one();
  }

  bool Empty() {
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_queue.empty();
  }

  bool Full() {
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_queue.size() >= m_maxSize;
  }

  size_t Size() {
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_queue.size();
  }

  size_t Count() {
    return m_queue.size();
  }

private:
  bool IsFull() const {
    return m_queue.size() >= m_maxSize;
  }

  bool IsEmpty() const {
    return m_queue.empty();
  }

private:
  std::list<T> m_queue; // task buffer
  std::mutex m_mutex; // mutex for condition
  std::condition_variable_any m_notEmpty; // not empty condition
  std::condition_variable_any m_notFull;  // not full condition
  int m_maxSize;  // queue max size
};
