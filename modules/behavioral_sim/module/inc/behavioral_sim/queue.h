#ifndef _BM_QUEUE_H_
#define _BM_QUEUE_H_

#include <deque>
#include <mutex>
#include <condition_variable>


using std::deque;
using std::mutex;
using std::condition_variable;
using std::unique_lock;

/* TODO: implement non blocking behavior */

template <class T>
class Queue {
public:
  enum WriteBehavior { WriteBlock, WriteReturn };
  enum ReadBehavior { ReadBlock, ReadReturn };

public:
  Queue(size_t capacity,
	WriteBehavior wb = WriteReturn, ReadBehavior rb = ReadBlock)
    : capacity(capacity), wb(wb), rb(rb) {}

  void push_front(const T &item) {
    unique_lock<mutex> lock(q_mutex);
    while(!is_not_full())
      q_not_full.wait(lock);
    queue.push_front(item);
    lock.unlock();
    q_not_empty.notify_one();
  }

  void push_front(T &&item) {
    unique_lock<mutex> lock(q_mutex);
    while(!is_not_full())
      q_not_full.wait(lock);
    queue.push_front(std::move(item));
    lock.unlock();
    q_not_empty.notify_one();
  }

  void pop_back(T* pItem) {
    unique_lock<mutex> lock(q_mutex);
    while(!is_not_empty())
      q_not_empty.wait(lock);
    *pItem = std::move(queue.back());
    queue.pop_back();
    lock.unlock();
    q_not_full.notify_one();
  }

  size_t size() {
    q_mutex.lock();
    return queue.size();
    q_mutex.unlock();
  }

  Queue(const Queue &) = delete;
  Queue &operator =(const Queue &) = delete;

private:
  bool is_not_empty() const { return queue.size() > 0; }
  bool is_not_full() const { return queue.size() < capacity; }

  const size_t capacity;
  deque<T> queue;
  WriteBehavior wb;
  ReadBehavior rb;

  mutex q_mutex;
  condition_variable q_not_empty;
  condition_variable q_not_full;
};

#endif