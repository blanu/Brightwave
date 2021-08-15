#include <U8g2lib.h>

#include "Encoder.h"
#include "Buffer.h"

const int marginX = 1;
const int marginY = 1;
const int fontHeight = 2;
const int fontWidth = 8;

struct Display
{
  Counter counter;

  int cursorX = marginX;
  int cursorY = marginY;
  
  // oled
  U8X8_SSD1309_128X64_NONAME0_4W_SW_SPI oled = U8X8_SSD1309_128X64_NONAME0_4W_SW_SPI(/* clock=*/8, /* data=*/10, /* cs=*/7, /* dc=*/9, /* reset=*/30);  
  int lastScreenUpdate = 0;

  void setup()
  {
    oled.setFont(u8x8_font_chroma48medium8_r);
    oled.begin();
  }

  void update(EncoderMode mode, int hold, int toneControl, int tone, Buffer buffer)
  {
    int now = millis();
    int duration = now - lastScreenUpdate;
  
    if (duration > 1000)
    {    
      resetDisplay();
  
      println("Brightwave");
      println("CTRL3", toneControl);
      println("Tone", tone);
        
      switch(mode)
      {
        case START_COUNT_MODE:
          counter.startSnapshot();
        case COUNT_MODE:
          println("buffer len", buffer.appendIndex);
          println("counter.buffer len", counter.buffer.appendIndex);
          if (counter.buffer.appendIndex > 48) {
            println("<<<<<");
            for (int i = counter.buffer.appendIndex - 48; i < counter.buffer.appendIndex; i++) {
              printbuf(counter.buffer.buffer[i]);            
            }
            println("");
            println("<<<<<");
          }
          println("Sampling", counter.frequency);
          println("Sampling fft", counter.frequency_fft);
          
          if (counter.checkSnapshot())
          {
            drawSnapshot(counter.audioSnapshot, counter.snapshotSize);
            counter.completeSnapshot();
          }
          else
          {
            counter.startSnapshot();      
          }
          break;
        case START_HOLD_MODE:
          counter.completeSnapshot();
        case HOLD_MODE:
          println("Holding", hold);
          break;
      }
    
      lastScreenUpdate = now;
    }
  }

  void drawSnapshot(float *snapshot, size_t size)
  {
    const int boxWidth = 2;
    const int boxMaxHeight = 10;
    const float maxValue = 10;
    const float minValue = -10;
    
    for(int index = 0; index < size; index++)
    {
      float value = snapshot[index];
  
      if (value == 0)
      {
        
      }
      else if (value > 0)
      {
        
      }
      else // value < 0
      {
        
      }
    }
  }  

  void resetDisplay()
  {
    cursorX = 0;
    cursorY = 0;
  
    oled.clearDisplay();
  }
  
  void displayBarGraph(float *data, size_t size)
  {
    for(int index = 0; index < size; index++)
    {
      
    }
  }  

  // Convenience print functions
  
  void println(String label, float number)
  {
    String outputString = String(label);
    outputString.concat(" ");
    
    String floatString = String(number, 1); // Convert float to string with one decimal place.
    outputString.concat(floatString);
    
    println(outputString);
  }
  
  void println(String label, int number)
  {
    String outputString = String(label);
    outputString.concat(" ");
  
    String intString = String(number);
    outputString.concat(intString);
    
    println(outputString);
  }
  
  void println(String string)
  {
    const char *cstring = string.c_str();
    println(cstring);
  }
  
  // Serious print functions
  
  void print(const char *cstring)
  {
    oled.drawString(cursorX, cursorY, cstring);
    cursorX += strlen(cstring) * fontWidth;
    // Do not update cursorY
  
    Serial.print(cstring);
  }
  
  void println(const char *cstring)
  {
    oled.drawString(cursorX, cursorY, cstring);
    cursorX = marginX;
    cursorY += fontHeight;
  
    Serial.println(cstring);
  }  

  void printbuf(float number)
  {
    String floatString = String(number, 1); // Convert float to string with one decimal place.
    Serial.print(floatString);
    Serial.print(" ");
  }  
};
