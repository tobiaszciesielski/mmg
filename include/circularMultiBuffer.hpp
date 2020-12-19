#include <vector>
#include <limits>
using std::vector;

template <typename T>
class Buffer {
private:
  size_t head;
  size_t tail;
  size_t expectedFrameIndex;
  bool isFull;
  size_t packagesCount;
  size_t packageSize;
  size_t frameSize;  
  T NaN = std::numeric_limits<T>::max();
  std::vector<std::vector<std::vector<T>>> buffer;
  std::vector<T> invalidVector;

private:
  void pushHead() {
    head = (head+1) % packagesCount;

    // if package index is higher than packages count than buffer overflows 
    if(isFull) tail = (tail+1) % packagesCount;
    isFull = (head == tail);
  }

  void pushTail() {
    tail = (tail+1) % packagesCount;
  }

  void fillWithNull(size_t &frame_index) {
    if (frame_index > expectedFrameIndex) { 

      // fill gap with NaN (max of value of data type)
      for (size_t i = expectedFrameIndex; i < frame_index; i++) {
        buffer[head][i] = invalidVector;
      }
    } else { 
      int pos = 0; 
      int steps = packageSize - abs(int(expectedFrameIndex - frame_index));
      
      // fill gap with NaN (max of value of data type)
      for (int j = 0; j < steps; j++) {
        pos = (expectedFrameIndex + j) % packageSize;
        
        // if we we fill package go to next one
        if (pos == 0) {
          pushHead();
        }

        buffer[head][pos] = invalidVector;
      }

      // if incoming frame is 0, we dont want to overwrite package filled with NaN's.
      // we switch to new package
      if (frame_index == 0) {
        pushHead();
      }
    }
    expectedFrameIndex = frame_index;
  }

public:
  Buffer(size_t packages_count, size_t package_size, size_t frame_size):   
    head(0),
    tail(0),
    expectedFrameIndex(0),
    isFull(false),
    packagesCount(packages_count),
    packageSize(package_size),
    frameSize(frame_size) {
      buffer = vector<vector<vector<T>>>(packages_count, vector<vector<T>>(package_size, vector<T>(frame_size)));
      invalidVector = vector<T>(frameSize, NaN);
    }

  // insert value into passed position
  void insert(vector<T>& value, size_t frame_index) {   
    if (expectedFrameIndex != frame_index) {
      fillWithNull(frame_index);
    }
    
    buffer[head][frame_index] = value;
    expectedFrameIndex++;

    // if frame index is higher than package size, 
    // current package is full so pick next package
    if (expectedFrameIndex > packageSize-1) {
      pushHead();
    }
    
    // frame index is never higher than package size because package consists of frames
    expectedFrameIndex%=packageSize;
  }

  // get package (column)
  void get(T** table) {
    if (isEmpty()) return;

    for (size_t i = 0; i < packageSize; i++) {
      for (size_t j = 0; j < frameSize; j++) {
        table[i][j] = buffer[tail][i][j];
      }
    }
    
    pushTail();
    isFull = false;
  }


  // delete all values from buffer
  void clear() {
    expectedFrameIndex = 0;
    head = tail;
    isFull = false;
  }

  // check if buffer is empty
  bool isEmpty() {
    return (!isFull && (head == tail));
    return false;
  }

  // check if buffer is filled
  bool isFilled() {
    return isFull;
  }

  const size_t getPackagesCount() {
    return packagesCount;
  }

  const size_t getFrameSize() {
    return frameSize;
  }

  const size_t getPackageSize() {
    return packageSize;
  }

  // get count of filled rows
  const size_t getCapacity() {
    size_t capacity = packagesCount;
    if(!isFull) {
      if(head >= tail)
        capacity = head - tail;
      else
        capacity = packagesCount + head - tail;
    }
    return capacity;
  }
};