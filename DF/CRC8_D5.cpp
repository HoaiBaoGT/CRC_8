#include "CRC8_D5.h"
CRC8_D5::CRC8_D5() {}
unsigned long CRC8_D5::message_generate(unsigned long data) {
  unsigned long odd = data << SIZE_CRC;
  for (int i = (SIZE_MESSAGE - 1); i >= SIZE_CRC; i--) {
    unsigned long check = odd >> i;
    if (check != 0) {
      unsigned long divisor = CRC_KEY << (i - SIZE_CRC);
      odd = odd ^ divisor;
    }
  }
  return (data << SIZE_CRC) | odd;
}
bool CRC8_D5::message_check(unsigned long message) {
  if ((message >> SIZE_CRC) == 0) {
    return false;
  }
  unsigned long odd = message;
  for (int i = (SIZE_MESSAGE - 1); i >= SIZE_CRC; i--) {
    unsigned long check = odd >> i;
    if (check != 0) {
      unsigned long divisor = CRC_KEY << (i - SIZE_CRC);
      odd = odd ^ divisor;
    }
  }
  if (odd == 0) {
    return true;
  }
  return false;
}
unsigned long CRC8_D5::get_data(unsigned long message){
  return message >> SIZE_CRC;
}
