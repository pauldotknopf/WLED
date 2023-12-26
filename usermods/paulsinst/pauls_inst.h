#pragma once

#include "wled.h"
#include "midi.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "scene.h"
#include "animation_chase.h"
#include "common.h"

#ifdef USERMOD_AUDIOREACTIVE
  #error You can not use paulsinst with audioreactive
#endif

#define NUM_GEQ_CHANNELS 16 // number of frequency channels. Don't change !!

uint16_t mode_synth(void);

// Effect parameters = Synth@!,Duty cycle
// Colors = !,!
// Palette = 
// Flags = 01
// Defaults =
static const char _data_FX_MODE_SYNTH[] PROGMEM = "ASynth";

//class name. Use something descriptive and leave the ": public Usermod" part :)
class PaulsInst : public Usermod {

  private:
    Midi* _midi;
    int _counter;
    Scene* _scene;
    Chase* _chase1;
    Chase* _chase2;
    Chase* _chase3;
    Chase* _chase4;
    Chase* _chase5;
    Chase* _chase6;

    // umdata
    // 2 - 255
    float _volumeSmth = 0.0f; // either sampleAvg or sampleAgc depending on soundAgc; smoothed sample
    // 2-255
    int16_t _volumeRaw = 0; // either sampleRaw or rawSampleAgc depending on soundAgc
    // 1250 const
    float _myMagnitude =0.0f; // FFT_Magnitude, scaled by multAgc
    // 16 channels, each 1-255
    uint8_t _fftResult[NUM_GEQ_CHANNELS] = {0};
    // period sample peaks
    bool _samplePeak = false;
    uint8_t _maxVol = 31; // Reasonable value for constant volume for 'peak detector', as it won't always trigger (deprecated)
    uint8_t _binNum = 8;
    // audioreactive constrains to 1.0f, 11025.0f
    // sound simulators range from 21.1200, 8149.1300
    float _FFTMajorPeak = 1.0f; 

  public:

    void print(PLA_RGBA color)
    {
      // Serial.print("r:");
      // Serial.println(color.r);
      // Serial.print("g:");
      // Serial.println(color.g);
      // Serial.print("b:");
      // Serial.println(color.b);
      // Serial.print("a:");
      // Serial.println(color.a);
    }

    PaulsInst()
      : _midi(NULL),
      _counter(0),
      _scene(NULL),
      _chase1(new Chase(10, PLA_RGBA(RED | 0xFF000000), true)),
      _chase2(new Chase(10, PLA_RGBA(BLUE | 0xFF000000), false))
    {
      print(PLA_RGBA(RED | 0xFF000000));
      print(PLA_RGBA(BLUE | 0xFF000000));
    }

    static PaulsInst* instance()
    {
      static PaulsInst instance;
      return &instance;
    }

    void ProcessMidiCmd(const MidiCmd &cmd)
    {
      switch(cmd.cmdType)
      {
        case MIDI_CMD_NOTE_OFF:
          // Serial.print(">note_off:");
          // Serial.println(cmd.note.note);
          // Serial.print(">velocity_off:");
          // Serial.println(cmd.note.velocity);
          // Serial.print(">noteKnob_off:");
          // Serial.println(cmd.note.note == 69);
          // Serial.print(">noteBump_off:");
          // Serial.println(cmd.note.note == 39);
          break;
        case MIDI_CMD_NOTE_ON:
          // Serial.print(">note:");
          // Serial.println(cmd.note.note);
          // Serial.print(">velocity:");
          // Serial.println(cmd.note.velocity);
          // Serial.print(">noteKnob:");
          // Serial.println(cmd.note.note == 69);
          // Serial.print(">noteBump:");
          // Serial.println(cmd.note.note == 39);
          break;
        case MIDI_CMD_CONTROLLER_CHANGE:
          switch(cmd.controllerChange.byte1)
          {
            case 1: {
              Serial.print(">tilt:");
              Serial.println(cmd.controllerChange.byte2);
              _FFTMajorPeak = map(cmd.controllerChange.byte2, 0, 127,  21.1200, 8149.1300);
              int intensity = map(cmd.controllerChange.byte2, 0, 127, 0, 255);
              for (byte i = 0; i < strip.getSegmentsNum(); i++) {
                Segment& seg = strip.getSegment(i);
                seg.speed = intensity;
              }
            }
              break;
            case 2:
              // Serial.print(">shake:");
              // Serial.println(cmd.controllerChange.byte2);
              break;
            case 74:
              // Serial.print(">radiate:");
              // Serial.println(cmd.controllerChange.byte2);
              break;
            case 112:
              // Serial.print(">knob:");
              // Serial.println(cmd.controllerChange.byte2);
              break;
            case 113:
              // Serial.print(">move:");
              // Serial.println(cmd.controllerChange.byte2);
              break;
            default:
              // Serial.print(">unknownController1:");
              // Serial.println(cmd.controllerChange.byte1);
              // Serial.print(">unknownController2:");
              // Serial.println(cmd.controllerChange.byte2);
              break;
          }
          break;
        case MIDI_CMD_CHANNEL_PRESSURE:
          // Serial.print(">channelPressure:");
          // Serial.println(cmd.pressure.pressure);
          break;
        case MIDI_CMD_PITCH_BEND:
          // Serial.print(">pitchBend:");
          // Serial.println(cmd.bend.bend);
          break;
        default:
          break;
      }
    }

    static void midiCmdCallback(const MidiCmd &cmd)
    {
      PaulsInst::instance()->ProcessMidiCmd(cmd);  
    }

    void setup()
    {
      um_data = new um_data_t;
      um_data->u_size = 8;
      um_data->u_type = new um_types_t[um_data->u_size];
      um_data->u_data = new void*[um_data->u_size];
      um_data->u_data[0] = &_volumeSmth;      //*used (New)
      um_data->u_type[0] = UMT_FLOAT;
      um_data->u_data[1] = &_volumeRaw;      // used (New)
      um_data->u_type[1] = UMT_UINT16;
      um_data->u_data[2] = _fftResult;        //*used (Blurz, DJ Light, Noisemove, GEQ_base, 2D Funky Plank, Akemi)
      um_data->u_type[2] = UMT_BYTE_ARR;
      um_data->u_data[3] = &_samplePeak;      //*used (Puddlepeak, Ripplepeak, Waterfall)
      um_data->u_type[3] = UMT_BYTE;
      um_data->u_data[4] = &_FFTMajorPeak;   //*used (Ripplepeak, Freqmap, Freqmatrix, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
      um_data->u_type[4] = UMT_FLOAT;
      um_data->u_data[5] = &_myMagnitude;   // used (New)
      um_data->u_type[5] = UMT_FLOAT;
      um_data->u_data[6] = &_maxVol;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
      um_data->u_type[6] = UMT_BYTE;
      um_data->u_data[7] = &_binNum;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
      um_data->u_type[7] = UMT_BYTE;

      strip.addEffect(255, &mode_synth, _data_FX_MODE_SYNTH);

      Serial1.begin(31250, SERIAL_8N1, 0, 4);
      _midi = new Midi(&Serial1);
      _midi->setCallback(&PaulsInst::midiCmdCallback);
    }

    void connected()
    {

    }

    void loop()
    {
      _counter++;
      if(_counter == 200)
      {
        _counter = 0;
        _samplePeak = !_samplePeak;
      }
      if(_midi)
      {
        _midi->tick();
      }
    }

    void renderScene()
    {
      if(_scene != NULL)
      {
        if(_scene->length() != SEGMENT.length())
        {
          delete _scene;
          _scene = NULL;
        }
      }

      if(!_scene)
      {
        _scene = new Scene(SEGMENT.length());
        _scene->addAnimation(_chase1);
        _scene->addAnimation(_chase2);
      }

      if(_scene->render())
      {
        for(auto i = 0; i < SEGMENT.length(); i++)
        {
          SEGMENT.setPixelColor(i, _scene->pixels()[i].mergeAlphaAsBlackBackground().raw32);
        }
      }
    }

    bool getUMData(um_data_t **data)
    {
      if (!data) return false;
      *data = um_data;
      return true;
    }

    uint16_t getId()
    {
      return USERMOD_ID_AUDIOREACTIVE;
    }
};

#define PALETTE_SOLID_WRAP   (strip.paletteBlend == 1 || strip.paletteBlend == 3)
#define PALETTE_MOVING_WRAP !(strip.paletteBlend == 2 || (strip.paletteBlend == 0 && SEGMENT.speed == 0))

static byte currentAlpha = 0;
uint16_t mode_synth(void) {
  SEGMENT.fill((uint32_t)CRGB(CRGB::White));
  //PaulsInst::instance()->renderScene();
  byte alpha = currentAlpha;
  for (uint16_t i =0; i < SEGMENT.length(); i++) {
    CRGB(CRGB::White).fadeLightBy(alpha);
    SEGMENT.fadePixelColor(i, alpha);
    alpha++;
  }
  currentAlpha++;
  return FRAMETIME;
}