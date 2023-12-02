#pragma once

#include "wled.h"

#ifdef USERMOD_AUDIOREACTIVE
  #error You can not use paulsinst with audioreactive
#endif

//class name. Use something descriptive and leave the ": public Usermod" part :)
class PaulsInst : public Usermod {

  private:

  public:
  
    void setup()
    {

    }

    void connected()
    {
      
    }


    void loop()
    {

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
