#ifndef BUFFER_INCLUDE
#define BUFFER_INCLUDE

struct Buffer
{
  static const int bufferSize = 4096;
  float buffer[bufferSize];
  int appendIndex = 0;

  void append(float *values, size_t size)
  {
    for(int index = 0; index < size; index++)
    {
      if (appendIndex >= bufferSize)
      {
        return;
      }
      
      buffer[appendIndex++] = values[index];
    }
  }

  bool isFull()
  {
    return appendIndex == bufferSize;
  }
};

#endif
