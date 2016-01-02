#ifndef CRC_H
#define CRC_H


#include <stdint.h>

namespace utils
{
namespace comm
{

extern bool crc_check(char  *b, uint32_t size);

extern unsigned short crc_calc(char *b, uint32_t size);

extern unsigned short crc_update(unsigned short crc, char data);

}
}

#endif
