#pragma once

#include "wled.h"

// a dummy "ignore" state for commands which
//  we wish to ignore.
#define MIDI_IGNORE 0x00

#define MIDI_STATE_BYTE1 0x00
#define MIDI_STATE_BYTE2 0x01
#define MIDI_STATE_BYTE3 0x02

#define MIDI_CMD_NOTE_OFF 0x80
#define MIDI_CMD_NOTE_ON 0x90
#define MIDI_CMD_KEY_PRESSURE 0xA0
#define MIDI_CMD_CONTROLLER_CHANGE 0xB0
#define MIDI_CMD_PROGRAM_CHANGE 0xC0
#define MIDI_CMD_CHANNEL_PRESSURE 0xD0
#define MIDI_CMD_PITCH_BEND 0xE0

typedef void (*MidiEventCallbackFunction)(const byte note /*0 - 127*/, const byte velocity /*0 - 127*/);

//class name. Use something descriptive and leave the ": public Usermod" part :)
class Midi {

  private:
    Stream* _stream;
    MidiEventCallbackFunction _callbackFunction;
    byte _note;
    byte _lastCommand;
    byte _state;
    byte _parameter1;
    byte _parameter2;
    byte _channel;

  public:
  
    Midi(Stream* stream)
      : _stream(stream),
      _callbackFunction(NULL)
    {
      _stream = stream;
    }

    void setCallback(MidiEventCallbackFunction callbackFunction)
    {
      _callbackFunction = callbackFunction;
    }

    void tick()
    {
      while (_stream->available())
      {
          // read the incoming byte:
          byte incomingByte = _stream->read();
          Serial.print("got a byte! ");
          Serial.print(incomingByte);
          Serial.println();
          // Command byte?
          if (incomingByte & 0b10000000)
          {
              _channel = incomingByte & 0x0F;
              _lastCommand = incomingByte & 0xF0;
              _state = MIDI_STATE_BYTE1; // Reset our state to byte1.

              // Serial.print("command byte: ");
              // Serial.print(_lastCommand);
              // Serial.println();
          }
          else if (_state == MIDI_STATE_BYTE1)
          {
              _parameter1 = incomingByte;
              _state = MIDI_STATE_BYTE2;

              if(_lastCommand == MIDI_CMD_CHANNEL_PRESSURE)
              {
                  // Serial.print("channel pressure: ");
                  // Serial.print(_channel);
                  // Serial.print(" : ");
                  // Serial.print(_parameter1);
                  // Serial.println();
              }
          }
          else if(_state == MIDI_STATE_BYTE2)
          {
              _parameter2 = incomingByte;
              _state = MIDI_STATE_BYTE3;

              if(_lastCommand == MIDI_CMD_CONTROLLER_CHANGE)
              {
                  // 1 tilt /2 shake / 112
                  if(_parameter1 == 112){
                  // Serial.print("controller change: ");
                  // Serial.print(_channel);
                  // Serial.print(" : ");
                  // Serial.print(_parameter1);
                  // Serial.print(" : ");
                  // Serial.print(_parameter2);
                  // Serial.println();
                  }
              }

              if(_lastCommand == MIDI_CMD_PITCH_BEND)
              {
                  // Serial.print("pitch bend change: ");
                  // short bend = (_parameter2 << 7 | _parameter1) - 8192;
                  // Serial.print(bend);
                  // Serial.println();
              }

              if(_lastCommand == MIDI_CMD_NOTE_ON)
              {
                  if(_callbackFunction)
                  {
                      _callbackFunction(_parameter1, _parameter2);
                  }
              }
          }
      }
    }
};
