const int maxTones = 16;
const int maxThresholds = maxTones + 1;
float remaps[maxTones] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
//float thresholds[maxThresholds] = {2.015, 2.14, 2.265, 2.39, 2.515, 2.64, 2.765, 2.89, 3.015, 3.14, 3.265, 3.39, 3.515, 3.64, 3.765, 3.89};
//float thresholds[maxThresholds] = {0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960};
float thresholds[maxThresholds] = {220, 240, 270, 290, 320, 340, 380, 390, 460, 480, 500, 530, 550, 585, 600, 610};

struct Remapper
{
  float remap(float input)
  {
    int mappingIndex = findMapping(input);
  
    float mapping = remaps[mappingIndex];
    return mapping;
  }
  
  int findMapping(float input)
  {
    // Constrain to within lowest lower bound, inclusive
    if (input < thresholds[0])
    {
      return 0;
    }
  
    // Constrain to within higher upper bound, exclusive
    if (input >= thresholds[maxThresholds - 1])
    {
      return maxThresholds - 1;
    }
  
    for(int thresholdIndex = 0; thresholdIndex < maxThresholds - 1; thresholdIndex++)
    {
      float lowerBound = thresholds[thresholdIndex];
      float upperBound = thresholds[thresholdIndex];
  
      if (input >= lowerBound && input < upperBound)
      {
        return thresholdIndex;
      }
    }
  
    return maxThresholds - 1;
  }
};
