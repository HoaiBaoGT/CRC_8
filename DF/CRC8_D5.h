#ifndef _CRC8_D5_
#define _CRC8_D5_
/*
  TranHoaiBao: Fuck the #ifndef CRC8
  that can't have the same name as class CRC8
  for god know reason. Cost me 2 days for this.
*/
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class CRC8_D5 {
  public:
    CRC8_D5();
    unsigned long message_generate(unsigned long data);
    bool message_check(unsigned long message);
    unsigned long get_data(unsigned long message);
  private:
    static const int SIZE_MESSAGE = 32;
    static const int SIZE_CRC = 8;
    static const unsigned long CRC_KEY = 0x1D5;
};
#endif
