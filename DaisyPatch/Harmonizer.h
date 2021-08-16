struct Harmonizer
{
  float sampleRate;
  
  Oscillator fundamental;
  Oscillator fourth;
  Oscillator fifth;
  Oscillator octave;

  void start(float frequency)
  {
    fundamental.Init(sampleRate);
    fundamental.SetAmp(.7);    
    fundamental.SetWaveform(0);
    fundamental.SetFreq(frequency);

    fourth.Init(sampleRate);
    fourth.SetAmp(.7);    
    fourth.SetWaveform(0);
    fourth.SetFreq(frequency * (3.0/2.0));

    fifth.Init(sampleRate);
    fifth.SetAmp(.7);    
    fifth.SetWaveform(0);
    fifth.SetFreq(frequency * (4.0/3.0));

    octave.Init(sampleRate);
    octave.SetAmp(.7);    
    octave.SetWaveform(0);
    octave.SetFreq(frequency * 2.0);
  }

  void setFrequency(float frequency)
  {
    fundamental.SetFreq(frequency);    
    fourth.SetFreq(frequency * (3.0/2.0));    
    fifth.SetFreq(frequency * (4.0/3.0));    
    octave.SetFreq(frequency * 2.0);    
  }

  void process(float **out, size_t size)
  {
    for(int index = 0; index < size; index++)
    {
      out[0][index] = fundamental.Process();
      out[1][index] = fourth.Process();
      out[2][index] = fifth.Process();
      out[3][index] = octave.Process();
    }
  }
};
