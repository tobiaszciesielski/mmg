#include <iostream>
#include <vector>
#include <chrono>

template <typename T>
class CircularBuffer {
private:
  std::vector<std::vector<T>> buffer;
  size_t head;
  size_t tail;
  size_t rows;
  size_t cols;
  size_t expectedRow;
  bool isFull;
  T type_maximum = std::numeric_limits<T>::max();

private:
  void pushHead() {
    head = (head+1) % cols;

    if(isFull) tail = (tail+1) % cols;
    isFull = (head == tail);
  }

  void pushTail() {
    tail = (tail+1) % cols;
  }

  void fillWithZeros(size_t &row) {
    int steps = abs(row-expectedRow);
    int from, to;
    if (row > expectedRow) {
      // fill the hole with 0
      from = expectedRow;
      to = row;
      
      for(int i = from; i < to; i++) {
        buffer[i][head] = type_maximum;
      }
    } else if (row < expectedRow) {
      // fill the rest with 0 and move head
      from = expectedRow;
      to = rows;

      // -1 because we iterate from 0
      steps = rows-steps-1; 
      int pos;
      for(int i = 0; i < steps; i++) { 
        pos = (from + i) % rows;
        if(pos == 0) pushHead();
        buffer[pos][head] = type_maximum;
      }
    }
    expectedRow = row;
}

public:
  CircularBuffer(size_t bufferRows, size_t bufferCols): 
    buffer(std::vector<std::vector<T>>(bufferRows)),
    head(0), 
    tail(0), 
    rows(bufferRows),
    cols(bufferCols),
    expectedRow(0),
    isFull(false) {
      for (size_t i = 0; i < bufferRows; i++) buffer[i] = std::vector<T>(bufferCols);      
    }

  // insert value into passed position
  void insert(T value, size_t row) {   
    if(row != expectedRow) 
      fillWithZeros(row);

    buffer[row][head] = value;
    expectedRow++;

    if(expectedRow == rows) {
      pushHead();
      expectedRow = 0;
    }
  }

  // get package (column)
  std::vector<T> get() {
    if (isEmpty()) return std::vector<T>(rows, T());
    
    std::vector<T> tmpVector(rows);
    for (int i = 0; i < rows; i++) {
      tmpVector[i] = buffer[i][tail];
    }

    pushTail();
    isFull = false;
    return tmpVector;
  }


  // delete all values from buffer
  void clear() {
    expectedRow = 0;
    head = tail;
    isFull = false;
  }

  // check if buffer is empty
  bool isEmpty() {
    return (!isFull && (head == tail));
  }

  // check if buffer is filled
  bool isFilled() {
    return isFull;
  }

  // get package length
  size_t getColumsCount() {
    return cols;
  }

  // get buffer length
  size_t getRowsCount() {
    return rows;
  }

  // get count of filled rows
  size_t getCapacity() {
    size_t capacity = cols;
    if(!isFull) {
      if(head >= tail)
        capacity = head - tail;
      else
        capacity = cols + head - tail;
    }
    return capacity;
  }
};