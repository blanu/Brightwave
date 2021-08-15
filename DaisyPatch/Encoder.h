#ifndef ENCODER_INCLUDE
#define ENCODER_INCLUDE

// Count and hold state machine
enum EncoderMode
{
  COUNT_MODE = 0,
  HOLD_MODE = 1,
  START_COUNT_MODE = 2,
  START_HOLD_MODE = 3,
};

#endif
