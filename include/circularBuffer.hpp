#include <memory>

// // C++11 doesnt support make_unique
// template<typename T, typename... Args>
// std::unique_ptr<T> make_unique(Args&&... args) {
//     return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
// }


template <typename T>
class CircularBuffer {

private:
  std::unique_ptr<T[]> buffer;
  size_t head;
  size_t tail;
  size_t size;
  bool isFull;

public:
  CircularBuffer(size_t bufferSize) : 
    buffer(std::make_unique<T[]>(bufferSize)), 
    head(0), 
    tail(0), 
    size(bufferSize),
    isFull(false) {}

  void insert(T value) {   
    buffer[head++] = value;        
    head %= size;
    
    if(isFull) tail = (tail+1) % size;
    isFull = (head == tail);
  }

  T get() {
    if (isEmpty()) return T();
    auto value = buffer[tail++];
    tail %= size;
    isFull = false;
    return value;
  }

  void clear() {
    head = tail;
    isFull = false;
  }

  bool isEmpty() {
    return (!isFull && (head == tail));
  }

  bool isFilled() {
    return isFull;
  }

  // get total size of buffer
  size_t getSize() {
    return size;
  }

  // get count of items stored in buffer
  size_t getCapacity() {
    size_t capacity = size;
    if(!isFull) {
      if(head >= tail)
        capacity = head - tail;
      else
        capacity = size + head - tail;
    }
    return capacity;
  }
};

