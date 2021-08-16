#ifndef WINDOWED_BUFFER_INCLUDE
#define WINDOWED_BUFFER_INCLUDE

struct WindowedBuffer
{
  static const int bufferSize = 4096;
  float buffer[bufferSize];
  int appendIndex = 0;

  void append(float *values, size_t size)
  {
    if (appendIndex >= bufferSize)
    {
        int extra = (appendIndex + size) - bufferSize;

        //memmove(buffer, buffer + extra, appendIndex);
        for (int i = 0; i < appendIndex; i++) {
            buffer[i] = buffer[i+extra];
        }
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
