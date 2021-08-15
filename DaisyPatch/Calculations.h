const int sampleAudioChannel = 0;

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

struct Counter
{
  float sampleRate;
  
  float lastSample = 0; // Value of previous sample, for calculating direction
  int lastCrossing = 0; // sample count at last zero-crossing
  int sampleCount = 0;  // current sample count
  float frequency = 0;
  float frequency_fft = 0;
  static const size_t snapshotSize = 48;
  float audioSnapshot[snapshotSize];
  bool snapshotNow = false;
  bool snapshotComplete = false;

  void reset()
  {
    lastSample = 0;
    lastCrossing = 0;
    sampleCount = 0;
    frequency = 0;
  }
  
  float count(float **in, size_t size)
  {
    int newFrequency = 0;
    
    for(int sampleNumber = 0; sampleNumber < size; sampleNumber++)
    {
      float nextSample = in[sampleAudioChannel][sampleNumber];

      if (snapshotNow)
      {
        audioSnapshot[sampleNumber] = nextSample;
      }
  
      if(sampleCount == 0) // We do not have a last sample.
      {
        lastSample = nextSample;
        sampleCount += 1;
      }
      else // We have a last sample.
      {
        if (checkCrossing(lastSample, nextSample))
        {
          if (lastCrossing == 0)
          {
            lastCrossing = sampleCount;
            continue;
          }
          else
          {
            int samplesSinceCrossing = sampleCount - lastCrossing;
  
            newFrequency = sampleRate / samplesSinceCrossing;
  
            lastCrossing = sampleCount;
          }
        }
  
        lastSample = nextSample;
        sampleCount += 1;
      }
    }

    if (snapshotNow)
    {
      snapshotNow = false;
      snapshotComplete = true;
    }
  
    frequency = newFrequency;
  }

  float fft(float **in, size_t size)
  {
      uint16_t samples = (uint16_t)size;
      arduinoFFT FFT = arduinoFFT();
      double *vReal = (double *)in[sampleAudioChannel];
      double vImag[samples];
  
      // zero imaginary part
      for (size_t i = 0; i < samples; i++)
      {
          vImag[i] = 0.0;
      }
  
      FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, samples);
      frequency_fft = FFT.MajorPeak(vReal, samples, sampleRate);
      return frequency_fft;
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

  void startSnapshot()
  {
    snapshotComplete = false;
    snapshotNow = true;
  }

  bool checkSnapshot()
  {
    return snapshotComplete;
  }

  void completeSnapshot()
  {
    snapshotNow = false;
    snapshotComplete = true;
  }
};
