#ifndef WINDOWED_BUFFER_INCLUDE
#define WINDOWED_BUFFER_INCLUDE

struct WindowedBuffer
{
  float buffer[bufferSize];
  int appendIndex = 0;

  void append(float *values, size_t size)
  {
    if (appendIndex + size >= bufferSize)
    {
      int extra = bufferSize - (appendIndex + size);

      memcpy(buffer, buffer + extra, extra);
      appendIndex -= extra;
    }
    
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
