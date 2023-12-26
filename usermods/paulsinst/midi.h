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

struct MidiCmdNote
{
  byte note;
  byte velocity;
};

struct MidiCmdPitchBend
{
  short bend;
};

struct MidiCmdControllerChange
{
  byte byte1;
  byte byte2;
};

struct MidiCmdPressure
{
  byte pressure;
};

struct MidiCmd
{
  byte cmdType;
  byte channel;
  union
  {
    MidiCmdNote note;
    MidiCmdPitchBend bend;
    MidiCmdControllerChange controllerChange;
    MidiCmdPressure pressure;
  };
};

typedef void (*MidiCmdCallbackFunction)(const MidiCmd &cmd);

// class name. Use something descriptive and leave the ": public Usermod" part :)
class Midi
{

private:
  Stream *_stream;
  MidiCmdCallbackFunction _callbackFunction;
  byte _state;
  byte _parameter1;
  byte _parameter2;
  MidiCmd _currentCmd;

public:
  Midi(Stream *stream)
      : _stream(stream),
        _callbackFunction(NULL)
  {
    _stream = stream;
  }

  void setCallback(MidiCmdCallbackFunction callbackFunction)
  {
    _callbackFunction = callbackFunction;
  }

  void tick()
  {
    while (_stream->available())
    {
      // read the incoming byte:
      byte incomingByte = _stream->read();

      // Command byte?
      if (incomingByte & 0b10000000)
      {
        _currentCmd.channel = incomingByte & 0x0F;
        _currentCmd.cmdType = incomingByte & 0xF0;
        _state = MIDI_STATE_BYTE1; // Reset our state to byte1.

        // Serial.print("command byte: ");
        // Serial.print( _currentCmd.cmdType);
        // Serial.println();
      }
      else if (_state == MIDI_STATE_BYTE1)
      {
        _parameter1 = incomingByte;
        _state = MIDI_STATE_BYTE2;

        if (_currentCmd.cmdType == MIDI_CMD_CHANNEL_PRESSURE)
        {
          _currentCmd.pressure.pressure = _parameter1;
          _callbackFunction(_currentCmd);
        }
      }
      else if (_state == MIDI_STATE_BYTE2)
      {
        _parameter2 = incomingByte;
        _state = MIDI_STATE_BYTE3;

        if (_currentCmd.cmdType == MIDI_CMD_CONTROLLER_CHANGE)
        {
          if(_callbackFunction) {
            _currentCmd.controllerChange.byte1 = _parameter1;
            _currentCmd.controllerChange.byte2 = _parameter2;
            _callbackFunction(_currentCmd);
          }
        }

        if (_currentCmd.cmdType == MIDI_CMD_PITCH_BEND)
        {
          if (_callbackFunction)
          {
            short bend = (_parameter2 << 7 | _parameter1) - 8192;
            _currentCmd.bend.bend = bend;
            _callbackFunction(_currentCmd);
          }
        }

        if (_currentCmd.cmdType == MIDI_CMD_NOTE_ON)
        {
          if (_callbackFunction)
          {
            _currentCmd.note.note = _parameter1;
            _currentCmd.note.velocity = _parameter2;
            _callbackFunction(_currentCmd);
          }
        }

        if (_currentCmd.cmdType == MIDI_CMD_NOTE_OFF)
        {
          if (_callbackFunction)
          {
            _currentCmd.note.note = _parameter1;
            _currentCmd.note.velocity = _parameter2;
            _callbackFunction(_currentCmd);
          }
        }
      }
    }
  }
};
