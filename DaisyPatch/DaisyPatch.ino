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

#include "Gate.h"
#include "Calculations.h"
#include "Display.h"
#include "Encoder.h"
#include "RemappingQuantizer.h"

DaisyHardware hw;
DaisyHardware patch;

float sampleRate = 0;

// Fundametal frequency hold is in Hz
float hold = 0.0;

const int maxTones = 16;
const int maxThresholds = maxTones + 1;
float remaps[maxTones] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
//float thresholds[maxThresholds] = {2.015, 2.14, 2.265, 2.39, 2.515, 2.64, 2.765, 2.89, 3.015, 3.14, 3.265, 3.39, 3.515, 3.64, 3.765, 3.89};
//float thresholds[maxThresholds] = {0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960};
float thresholds[maxThresholds] = {220, 240, 270, 290, 320, 340, 380, 390, 460, 480, 500, 530, 550, 585, 600, 610};

// Count and hold
Counter counter;
EncoderMode mode = HOLD_MODE; // Count/hold state machine
int toneControl;
int audioBufferSize;
int firstSample = 0;

// Gate inputs
Gate gate0 = {patch, 0}; // patch object, gate number 0
Gate gate1 = {patch, 1}; // patch object, gate number 1

// Display
Display display;

static void AudioCallback(float **in, float **out, size_t size)
{
  audioBufferSize = (int)size;

  if (in[0][0] > firstSample)
  {
    firstSample = in[0][0];
  }
  
  switch (mode)
  {
    case START_COUNT_MODE:
      // Wait for COUNT_MODE to avoid race conditions
      break;
    case START_HOLD_MODE:
      // Wait for HOLD_MODE to avoid race conditions
      break;
    case COUNT_MODE:
      counter.count(in, size);
      break;
    case HOLD_MODE:
      break;
    default:
      break;
  }
}

void setup()
{
  hw = DAISY.init(DAISY_PATCH, AUDIO_SR_48K);
  sampleRate = DAISY.get_samplerate();
  counter = {sampleRate};
  
  display = {counter};
  display.setup();

  DAISY.begin(AudioCallback);
}

void loop()
{
  updateControls();
  runCalculations();
  updateControlOutputs();
  display.update(mode, hold, toneControl);
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

  toneControl = analogRead(PIN_PATCH_CTRL_3);
}

void runCalculations()
{
  switch (mode)
  {
    case START_COUNT_MODE:
      counter.reset();
      break;
    case START_HOLD_MODE:
      hold = counter.frequency;
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
      analogWrite(PIN_PATCH_CV_1, scale(counter.frequency));
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
