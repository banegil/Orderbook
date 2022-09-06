
template<typename T>
class ThreadsafeQueue {
  queue<T> queue_;
  mutable mutex mutex_;
 
  // Moved out of public interface to prevent races between this
  // and pop().
  bool empty() const {
    return queue_.empty();
  }
 
 public:
  ThreadsafeQueue() = default;
  ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete ;
  ThreadsafeQueue& operator=(const ThreadsafeQueue<T> &) = delete ;
 
  virtual ~ThreadsafeQueue() { }
 
  unsigned long size() const {
    lock_guard<mutex> lock(mutex_);
    return queue_.size();
  }
  
  T front() {
    lock_guard<mutex> lock(mutex_);
    return queue_.front();
  }
 
  optional<T> pop() {
    lock_guard<mutex> lock(mutex_);
    if (queue_.empty()) {
      return {};
    }
    T tmp = queue_.front();
    queue_.pop();
    return tmp;
  }
 
  void push(const T &item) {
    lock_guard<mutex> lock(mutex_);
    queue_.push(item);
  }
  
  void print() {
    lock_guard<mutex> lock(mutex_);
    cout << queue_.front() << "\n";
  }
};
