#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include "trace.hpp"
#include <string>
#include <deque>
#include <stdint.h>

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
  ByteArray();
  ByteArray(const unsigned char *buffer, unsigned int len, bool bigendian = false);
  ByteArray(const ByteArray &ba);
  ByteArray(unsigned char x);
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
  void        put(const unsigned char *buffer, unsigned int len);
  /** Put nullptr terminated string */
  void        puts(std::string s);
  void        putf(float f);

  uint32_t    size() const;
  uint8_t     popc();
  uint16_t    popw();
  uint32_t    popl();
  uint64_t    popL();
  std::string pops();
  float       popf();
  void        pop_data(uint8_t *buffer, uint32_t len);
  void        pop(ByteArray &ba, uint32_t len);

  std::string to_string() const;

  void insert(uint8_t c);

 private:
  std::deque<unsigned char> data;
  bool bigendian;
  static utils::Logable log;
};

}
}

#endif
