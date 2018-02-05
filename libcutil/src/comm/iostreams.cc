#include "comm/iostreams.hpp"
#include <malloc.h>

namespace utils
{
namespace comm
{

int InputStream::read(model::ByteArray &ba, uint32_t length, int timeout)
{
  uint8_t *buf = (uint8_t *) malloc(length);
  ba.clear();
  int nr = read(buf, length, timeout);

  if(nr > 0)
    ba.put(buf, nr);

  free(buf);
  return nr;
}

// a cos theta + b sin theta = d
// a² cos² + b² sin² = d²
// a² cos² + a² sin² + (b² - a²) sin² = d²
// sin² = (d² - a²) / (b² - a²)
//

int InputStream::read(uint8_t *buffer, uint32_t length, int timeout)
{
  uint32_t i;
  int retcode;
  for(i = 0; i < length; i++)
  {
    retcode = getc(timeout);
    if(retcode == -1)
      return i;
    buffer[i] = (uint8_t) (retcode & 0xff);
  }
  return length;
}

void OutputStream::put(const model::ByteArray &ba)
{
  if(ba.size() == 0)
    return;
  unsigned char *tmp = (unsigned char *) malloc(ba.size());
  for(unsigned int i = 0; i < ba.size(); i++)
    tmp[i] = ba[i];
  write(tmp, ba.size());
  free(tmp);
}

unsigned short InputStream::getw()
{
  unsigned char c1 = (unsigned char) getc();
  unsigned char c2 = (unsigned char) getc();
  return (((unsigned short) c1) << 8) | ((unsigned short) c2 & 0x00ff);
}

void OutputStream::putw(unsigned short s)
{
  putc((s >> 8) & 0xff);
  putc(s & 0xff);
}

void InputStream::discard_rx_buffer()
{
  while(getc(1) != -1)
    ;
}

void OutputStream::put_string(std::string s)
{
  const char *ss = s.c_str();
  put_data(ss, s.size());
}

void InputStream::get_data(char *buffer, int len)
{
  for(int i = 0; i < len; i++)
    buffer[i] = getc();
}

int InputStream::get_line(std::string &res, int timeout)
{
  res = "";
  int retcode;
  for(;;)
  {
    retcode = getc(timeout);
    if(retcode == -1)
      return -1;
    char c[2];
    c[0] = (char) retcode;
    c[1] = 0;

    if(c[0] == '\n')
      return 0;

    res += std::string((const char *) &c[0]);

  }
  /* not reached */
  return 0;
}

void OutputStream::put_data(const void *buffer_, int len)
{
  const char *buffer = (const char *) buffer_;
  for(int i = 0; i < len; i++)
    putc(buffer[i]);
}

void OutputStream::write(const uint8_t *buffer, uint32_t len)
{
  for(uint32_t i = 0; i < len; i++)
    putc(buffer[i]);
}

}
}
