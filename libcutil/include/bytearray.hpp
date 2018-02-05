#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include <string>
#include <deque>
#include <stdint.h>
#include <stdio.h>
#include "journal.hpp"

#ifdef putc
#undef putc
#endif

namespace utils
{
namespace model
{

/** @brief A raw array of bytes */
class ByteArray
{
 public:

  void lis_fichier(FILE *fi, uint32_t lon);
  void ecris_fichier(FILE *fo);
  int ecris_fichier(const std::string &fn);
  int lis_fichier(const std::string &fn);

  ByteArray(int len);
  ByteArray();
  ByteArray(const unsigned char *buffer, unsigned int len, bool bigendian = false);
  ByteArray(const ByteArray &ba);
  //ByteArray(unsigned char x);
  ByteArray(std::string str, bool is_decimal = true);

  void operator =(const ByteArray &ba);
  const unsigned char &operator[](unsigned int i) const;
  unsigned char &operator[](unsigned int i);
  ByteArray operator +(const ByteArray &ba) const;
  ByteArray operator +(unsigned char c) const;
  bool operator ==(const ByteArray &ba) const;
  bool operator !=(const ByteArray &ba) const;

  void        clear();
  void        put(const ByteArray &ba);
  void        putc(uint8_t c);
  void        putw(uint16_t w);
  void        putl(uint32_t l);
  void        putL(uint64_t val);
  void        put(const void *buffer, unsigned int len);
  /** Put nullptr terminated string */
  void        puts_zt(std::string s);
  void        puts(std::string s);
  void        putf(float f);

  uint32_t    size() const;
  uint8_t     popc();
  uint16_t    popw();
  uint32_t    popl();
  uint64_t    popL();
  std::string pops();
  float       popf();
  void        pop_data(void *buffer, uint32_t len);
  void        pop(ByteArray &ba, uint32_t len);

  std::string to_string(bool hexa = false) const;

  void insert(uint8_t c);

  std::deque<unsigned char> data;
  bool bigendian;
};

}
}

#endif
