// Convert Hertz to 1V/Oct CV out
float scale(float hertz)
{
  float result = hertz;
  
  // Audible Hertz range from 20 to 20,000.
  // CV out ranges from 0 to 255.
  // Pin out   0 -     0 Hz -  -4V
  // Pin out   1 -     1 Hz -  -3V
  // Pin out   2 -     2 Hz -  -2V
  // Pin out   4 -     4 Hz -  -1V
  // Pin out   8 -     8 Hz -   0V
  // Pin out  16 -    16 Hz -   1V
  // Pin out  32 -    32 Hz -   2V
  // Pin out  64 -    64 Hz -   3V
  // Pin out 128 -   128 Hz -   4V
  // Pin out 255 -   255 Hz -   5V

  if (hertz < 0)
  {
    return 0;
  }

  if (hertz == 0)
  {
    return 0;
  }

  if (hertz < 1.0)
  {
    result = hertz;
    
    while (result < 1.0)
    {
      result *= 2;
    }

    return result;
  }

  if (hertz > 1.0)
  {
    result = hertz;
    
    while (result > 1.0)
    {
      result /= 2;
    }

    return result;
  }

  return hertz;
}

// Calculate zero crossings
bool checkCrossing(float lastSample, float nextSample)
{
  if (lastSample == nextSample) {return false;}

  if (lastSample < 0)
  {
    if (nextSample > 0)
    {
      // We have a zero-crossing!
      return true;
    }
  }
  else if (lastSample > 0)
  {
    if (nextSample < 0)
    {
      // We have a zero-crossing!
      return true;
    }
  }
  else // lastSample == 0
  {
    if (nextSample < 0)
    {
      // We have a zero-crossing!
      return true;
    }
    else if (nextSample > 0)
    {
      // We have a zero-crossing!
      return true;
    }
    else
    {
      return false;
    }
  }
}
