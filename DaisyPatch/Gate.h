struct Gate {
  DaisyHardware patch;
  int number;
  
  float lastValue = -1;

  bool RisingEdge()
  {
    float nextValue = patch.gateIns[number].State();

    // No last value, so a rising edge is not possible.
    if (lastValue == -1)
    {
      lastValue = nextValue;
      return false;
    }
    else
    {
      if (nextValue > lastValue)
      {
        lastValue = nextValue;

        return true;
      }
      else
      {
        lastValue = nextValue;

        return false;        
      }
    }
  }

  bool FallingEdge()
  {
    float nextValue = patch.gateIns[number].State();

    // No last value, so a rising edge is not possible.
    if (lastValue == -1)
    {
      lastValue = nextValue;
      return false;
    }
    else
    {
      if (nextValue < lastValue)
      {
        lastValue = nextValue;

        return true;
      }
      else
      {
        lastValue = nextValue;

        return false;        
      }
    }
  }  
};
