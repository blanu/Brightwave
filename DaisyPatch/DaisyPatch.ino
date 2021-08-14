// Title: Vco
// Description: Vco with amplitude, waveforms, fine tune, and pitch
// Hardware: Daisy Patch
// Author: Ben Sergentanis
// Diagram:
// https://raw.githubusercontent.com/electro-smith/DaisyExamples/master/patch/Vco/resources/Vco.png
//
// Inputs
// ------
// Audio Input 1 - Fundamental frequency sampler
// MIDI In - MIDI to CV + Tone Mapping to MIDI Out
// 
// Controls
// --------
// Gate In 1 - Sample/Hold - OR with Enc 1.
// Gate In 2 - Gate control for CV Out 1
// Gate Out 1 - Square wave at fundamental frequency
//
// Enc 1 - Sample/Hold - sample when pressed. Turning knob rotates through 16 holds.
//         Default value for empty hold taken from Ctrl 2.
//
// Ctrl 1 - When Audio Input 1 disconnected, Fundamental frequency direct override.
//        - When Audio Input 1 connected, fundamental frequency fine tune.
// Ctrl 2 - Default value for empty holds
// Ctrl 3 - Waveform
// Ctrl 4 - Amplitude
//
// SD card - holds tone mappings
//
// Outputs
// -------
// Audio Out 1 - Held waveform
// Audio Out 2 - FM modulated sine wave at fundamental frequency, FM controlled by Audio In 1
//               FM modulates between the fundamental and the tone at a rate specified by the audio input
// Audio Out 3 - FM modulated triangle wave at fundamental frequency, FM controlled by Audio In 3
//               FM modulates between the fundamental and the tone at a rate specified by the audio input
// Audio Out 4 - PWM wave at fundamental frequency, PWM controlled by Audio In 4
//
// MIDI Out - MIDI to CV + Tone Mapping from MIDI In
//
// CV Out 1 - Hold, 1V/Oct
// CV Out 2 - Multiplier: Fundamental Hold * Tone Ratio

#include "DaisyDuino.h"
#include <U8g2lib.h>

#include "Gate.h"
#include "Print.h"

enum EncoderMode
{
  SAMPLE_MODE = 0,
  HOLD_MODE = 1
};

const int maxHolds = 16;
const int sampleAudioChannel = 0;

const int marginX = 1;
const int marginY = 1;
const int fontHeight = 2;
const int fontWidth = 8;

int cursorX = marginX;
int cursorY = marginY;

DaisyHardware hw;

DaisyHardware patch;
Oscillator osc;

// oled
U8X8_SSD1309_128X64_NONAME0_4W_SW_SPI oled(/* clock=*/8, /* data=*/10,
                                           /* cs=*/7, /* dc=*/9, /* reset=*/30);

int num_waves, num_channels;

float sampleRate = 0;

// Fundametal frequency holds are in Hz
float holds[maxHolds] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int holdPos=0;

const int maxTones = 20;
const int maxThresholds = maxTones + 1;
float remaps[maxTones] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0};
float thresholds[maxThresholds] = {-10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

EncoderMode mode = HOLD_MODE;
EncoderMode lastMode = HOLD_MODE;

float lastSample = 0; // Value of previous sample, for calculating direction
int lastCrossing = 0; // sample count at last zero-crossing
int sampleCount = 0;  // current sample count
float frequency = 0;

Gate gate0 = {patch, 0};

int lastScreenUpdate = 0;

static void AudioCallback(float **in, float **out, size_t size) {
  float sig, freq, amp;
  size_t wave;

  // Read gate input
  bool gate0On = patch.gateIns[0].State() == 1;

  if (mode == SAMPLE_MODE)
  {
    sample(in, size);

    // Output CV Out 1 - Hold, 1V/Oct
    analogWrite(PIN_PATCH_CV_1, scale(frequency));    
  }
  else // mode == HOLD
  {
    // Output CV Out 1 - Hold, 1V/Oct
    if (holds[holdPos] == 0)
    {
      // Use default output from CTRL 1.
      analogWrite(PIN_PATCH_CV_1, analogRead(PIN_PATCH_CTRL_1));
    }
    else
    {
      // Output held value
      analogWrite(PIN_PATCH_CV_1, scale(holds[holdPos]));
    }    
  }
}

void setup() {  
  num_waves = Oscillator::WAVE_LAST - 1;
  hw = DAISY.init(DAISY_PATCH, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  sampleRate = DAISY.get_samplerate();

  osc.Init(sampleRate); // Init oscillator

  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.begin();

  DAISY.begin(AudioCallback);
}

void loop()
{
  hw.DebounceControls();

  if (analogRead(PIN_PATCH_CTRL_2) > 512)
  {
    mode = SAMPLE_MODE;

    switch (lastMode)
    {
      // Continue sampling
      case SAMPLE_MODE:
        break;
      // Start holding
      case HOLD_MODE:    
        holds[holdPos] = frequency;
        break;
    }    
  }
  else // CTRL2 <= 512
  {
    mode = HOLD_MODE;

    switch (lastMode)
    {
      // Start sampling
      case SAMPLE_MODE:
        lastSample = 0;
        lastCrossing = 0;
        sampleCount = 0;        
        break;
      // Continue holding
      case HOLD_MODE:    
        break;
    }        
  }

  lastMode = mode;
  
  int now = millis();
  int duration = now - lastScreenUpdate;

  if (duration > 1000)
  {
    resetDisplay();
    
    println("Brightwave");
  
    switch(mode)
    {
      case SAMPLE_MODE:
        println("Sampling %f", frequency);
        break;
      case HOLD_MODE:
        println("Holding %f", holds[holdPos]);
        break;
    }

    lastScreenUpdate = now;
  }
}

void sample(float **in, size_t size)
{
  for(int sampleNumber = 0; sampleNumber < size; sampleNumber++)
  {    
    float nextSample = in[sampleAudioChannel][sampleNumber];

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

          frequency = sampleRate / samplesSinceCrossing;
          
          lastCrossing = sampleCount;
        }
      }

      lastSample = nextSample;
      sampleCount += 1;
    }
  }
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

float remap(float input)
{
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

void resetDisplay()
{
  cursorX = 0;
  cursorY = 0;

  oled.clearDisplay();
}

void println(String string)
{
  const char *cstring = string.c_str();
  println(cstring);
}

void println(String label, float number)
{
  char outputString[100];
  const char *cstring = label.c_str();
  sprintf(outputString, cstring, number);
  println(outputString);
}

void println(String label, int number)
{
  char outputString[100];
  const char *cstring = label.c_str();
  sprintf(outputString, cstring, number);
  println(outputString);
}

void print(const char *cstring)
{
  oled.drawString(cursorX, cursorY, cstring);  
  cursorX += strlen(cstring) * fontWidth;
  // Do not update cursorY
}

void println(const char *cstring)
{
  oled.drawString(cursorX, cursorY, cstring);
  cursorX = marginX;
  cursorY += fontHeight;
}
