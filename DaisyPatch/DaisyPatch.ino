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
#include "Calculations.h"

// Count and hold state machine
enum EncoderMode
{
  COUNT_MODE = 0,
  HOLD_MODE = 1,
  START_COUNT_MODE = 2,
  START_HOLD_MODE = 3,
};

const int sampleAudioChannel = 0;

const int marginX = 1;
const int marginY = 1;
const int fontHeight = 2;
const int fontWidth = 8;

int cursorX = marginX;
int cursorY = marginY;

DaisyHardware hw;
DaisyHardware patch;

// oled
U8X8_SSD1309_128X64_NONAME0_4W_SW_SPI oled(/* clock=*/8, /* data=*/10,
                                           /* cs=*/7, /* dc=*/9, /* reset=*/30);

float sampleRate = 0;

// Fundametal frequency hold is in Hz
float hold = 0.0;

const int maxTones = 16;
const int maxThresholds = maxTones + 1;
float remaps[maxTones] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0};
float thresholds[maxThresholds] = {2.015, 2.14, 2.265, 2.39, 2.515, 2.64, 2.765, 2.89, 3.015, 3.14, 3.265, 3.39, 3.515, 3.64, 3.765, 3.89};

EncoderMode mode = HOLD_MODE; // Count/hold state machine
float lastSample = 0; // Value of previous sample, for calculating direction
int lastCrossing = 0; // sample count at last zero-crossing
int sampleCount = 0;  // current sample count
float frequency = 0;

// Gate inputs
Gate gate0 = {patch, 0}; // patch object, gate number 0
Gate gate1 = {patch, 1}; // patch object, gate number 1

int lastScreenUpdate = 0;

static void AudioCallback(float **in, float **out, size_t size)
{
  switch (mode)
  {
    case START_COUNT_MODE:
      // Wait for COUNT_MODE to avoid race conditions
      break;
    case START_HOLD_MODE:
      // Wait for HOLD_MODE to avoid race conditions
      break;
    case COUNT_MODE:
      count(in, size);
      break;
    case HOLD_MODE:
      break;
    default:
      break;
  }
}

void setup() {  
  hw = DAISY.init(DAISY_PATCH, AUDIO_SR_48K);
  sampleRate = DAISY.get_samplerate();

  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.begin();

  DAISY.begin(AudioCallback);
}

void loop()
{
  updateControls();
  runCalculations();
  updateControlOutputs();
  updateDisplay();
}

void updateControls()
{
  hw.DebounceControls();

  // CTRL 2 is left of center.
  if (analogRead(PIN_PATCH_CTRL_2) > 512)
  {
    switch (mode)
    {
      case COUNT_MODE:
        break;
      default:
        mode = START_COUNT_MODE;
        break;
    }
  }
  else // CTRL2 <= 512, so CTRL2 is right of center (or center).
  {
    switch (mode)
    {
      case HOLD_MODE:
        break;
      default:
        mode = START_HOLD_MODE;
        break;
    }
  }
}

void runCalculations()
{
  switch (mode)
  {
    case START_COUNT_MODE:
      lastSample = 0;
      lastCrossing = 0;
      sampleCount = 0;        
      break;
    case START_HOLD_MODE:
      hold = frequency;
      break;
     case HOLD_MODE:
       break;
     case COUNT_MODE:
       break;
     default:
       break;
  }  

  
}

// Updates CV and Gate outputs. Audio outputs are handled by AudioCallback()
void updateControlOutputs()
{
  switch (mode)
  {
    case START_COUNT_MODE:
      // Wait for COUNT_MODE to avoid race conditions
      break;
    case START_HOLD_MODE:
      // Wait for HOLD_MODE to avoid race conditions
      break;
    case COUNT_MODE:  
      // Output CV Out 1 - Current counted frequency scaled to 1V/Oct
      analogWrite(PIN_PATCH_CV_1, scale(frequency));    
      break;
    case HOLD_MODE:
      // Output CV Out 1
      if (hold == 0) // No hold frequency?
      {
        // Use default output from CTRL 1.
        analogWrite(PIN_PATCH_CV_1, analogRead(PIN_PATCH_CTRL_1));
      }
      else // We have a hold frequency
      {
        // Output held value scaled to 1V/Oct
        analogWrite(PIN_PATCH_CV_1, scale(hold));
      }    
      break;
    default:
      break;
  }  
}

void updateDisplay()
{
  int now = millis();
  int duration = now - lastScreenUpdate;

  if (duration > 1000)
  {
    resetDisplay();
    
    println("Brightwave");
  
    switch(mode)
    {
      case START_COUNT_MODE:
      case COUNT_MODE:
        println("Sampling %f", frequency);
        break;
      case START_HOLD_MODE:
      case HOLD_MODE:
        println("Holding %f", hold);
        break;
    }

    lastScreenUpdate = now;
  }
}

void count(float **in, size_t size)
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
