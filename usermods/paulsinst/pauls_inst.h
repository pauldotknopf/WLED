#pragma once

#include "wled.h"
#include "midi.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#ifdef USERMOD_AUDIOREACTIVE
  #error You can not use paulsinst with audioreactive
#endif

//class name. Use something descriptive and leave the ": public Usermod" part :)
class PaulsInst : public Usermod {

  private:
    Midi* _midi;
    int _counter;

  public:

    PaulsInst()
      : _midi(NULL),
      _counter(0)
    {

    }

    void setup()
    {
      Serial1.begin(31250, SERIAL_8N1, 0, 4);
      _midi = new Midi(&Serial1);
    }

    void connected()
    {

    }

    void loop()
    {
      _counter++;
      if(_counter == 20)
      {
        _counter = 0;
        Serial.println("test");
      }
      if(_midi) {
        _midi->tick();
      }
    }

    bool getUMData(um_data_t **data)
    {
      return false;
      // if (!data || !enabled) return false; // no pointer provided by caller or not enabled -> exit
      // *data = um_data;
      // return true;
    }

    uint16_t getId()
    {
      return USERMOD_ID_AUDIOREACTIVE;
    }
};
