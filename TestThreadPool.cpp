#include <iostream>
#include <thread>

#include "ThreadPool.hpp"

void TestThreadPool() {
  ThreadPool pool(2);

  std::thread t1([&pool]{
    for (int i = 0; i < 10; ++i) {
      auto tid = std::this_thread::get_id();
      pool.AddTask([tid, i]{
        // std::cout << "thread 1 tid: " << tid << " i: " << i << std::endl;
        printf("thread 1 tid: %u i: %d \n", tid, i);
      });
    }
  });

  std::thread t2([&pool]{
    for (int i = 0; i < 10; ++i) {
      auto tid = std::this_thread::get_id();
      pool.AddTask([tid, i]{
        // std::cout << "thread 2 tid: " << tid << " i: " << i << std::endl;
        printf("thread 2 tid: %u i: %d \n", tid, i);
      });
    }
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  // getchar();
  pool.Stop();
  t1.join();
  t2.join();
}

int main(void) {
  TestThreadPool();
  return 0;
}
