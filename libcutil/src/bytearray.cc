#include "bytearray.hpp"
#ifdef LINUX
#include <cstdio>
#endif
#include <stdio.h>
#include "cutil.hpp"
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>

namespace utils
{
namespace model
{

using namespace utils;




ByteArray::ByteArray(std::string str, bool is_decimal)
{
  const char *s = str.c_str();
  unsigned char current = 0;
  if(!str::is_deci(s[0]))
  {
    return;
  }
  while(strlen(s) > 0)
  {
      if(str::is_deci(s[0]))
      {
          current = current * 10 + (s[0] - '0');
      }
      else
      {
          if(strlen(s) > 1)
          {
              putc(current);
              current = 0;
          }
      }
      s++;
  }
  putc(current);
}

std::string ByteArray::to_string(bool hexa) const
{
  std::string s = "";

  if(data.size() == 0)
    return "(empty)";

  for(unsigned int i = 0; i < data.size(); i++)
  {
    char buf[10];
    if(hexa)
      sprintf(buf, "%02x", data[i]);
    else
      sprintf(buf, "%d", data[i]);
    s += std::string(buf);
    if(i + 1 < data.size())
      s += ".";
    /*if((i > 0) && ((i & 15) == 0))
      s += std::string("\n");*/
  }
  return s;
}

/*ByteArray::ByteArray(unsigned char x)
{
  putc(x);
}*/

const unsigned char &ByteArray::operator[](unsigned int i) const
{
  return data[i];
}

unsigned char &ByteArray::operator[](unsigned int i)
{
  return data[i];
}

ByteArray::ByteArray(/*bool bigendian*/)
{
  this->bigendian = false;
}

void ByteArray::lis_fichier(FILE *fi, uint32_t n)
{
  uint8_t *tmp = (uint8_t *) malloc(n);
  fread(tmp, 1, n, fi);
  for(auto i = 0u; i < n; i++)
    data.push_back(tmp[i]);
  free(tmp);
}

static int32_t fsize(std::string filename)
{
    struct stat st;

    if (stat(filename.c_str(), &st) == 0)
        return st.st_size;

    return -1;
}

int ByteArray::lis_fichier(const std::string &fn)
{
  FILE *fi = fopen(fn.c_str(), "rb");
  if(fi == nullptr)
  {
    erreur("Erreur lors de l'ouverture du fichier [%s] en lecture.", fn.c_str());
    return -1;
  }

  auto lon = fsize(fn);

  if(lon < 0)
    return -1;

  lis_fichier(fi, lon);

  fclose(fi);
  return 0;
}

int ByteArray::ecris_fichier(const std::string &fn)
{
  FILE *fo = fopen(fn.c_str(), "wb");
  if(fo == nullptr)
  {
    erreur("Erreur lors de l'ouverture du fichier [%s] en écriture.", fn.c_str());
    return -1;
  }
  ecris_fichier(fo);
  fclose(fo);
  return 0;
}

void ByteArray::ecris_fichier(FILE *fo)
{
  uint32_t n = data.size();
  uint8_t *tmp = (uint8_t *) malloc(n);
  auto ptr = data.begin();
  for(auto i = 0u; i < n; i++)
    tmp[i] = *ptr++;
  fwrite(tmp, 1, n, fo);
  free(tmp);
}

ByteArray::ByteArray(int len)
{
  this->bigendian = false;
  data.resize(len);
}

ByteArray::ByteArray(const unsigned char *buffer, unsigned int len, bool bigendian)
{
  this->bigendian = bigendian;
  put(buffer, len);
}

ByteArray::ByteArray(const ByteArray &ba)
{
  *this = ba;
}

void ByteArray::clear()
{
  data.clear();
}

bool ByteArray::operator ==(const ByteArray &ba) const
{
  return data == ba.data;
}

bool ByteArray::operator !=(const ByteArray &ba) const
{
  return data != ba.data;
}

void ByteArray::operator =(const ByteArray &ba)
{
  //clear();
  data = ba.data;
  //for(unsigned int i = 0; i < ba.size(); i++)
  //  putc(ba.data[i]);
}

ByteArray ByteArray::operator +(const ByteArray &ba) const
{
  ByteArray res(*this);
  for(unsigned int i = 0; i < ba.size(); i++)
    res.putc(ba[i]);
  return res;
}

ByteArray ByteArray::operator +(unsigned char c) const
{
  ByteArray res(*this);
  res.putc(c);
  return res;
}

void ByteArray::putc(uint8_t c)
{
  data.push_back(c);
}

void ByteArray::putw(uint16_t w)
{
  putc((uint8_t) (w & 0xff));
  putc((uint8_t) ((w >> 8) & 0xff));
}

void ByteArray::putl(uint32_t l)
{
  putw((uint16_t) (l & 0xffff));
  putw((uint16_t) ((l >> 16) & 0xffff));
}

void ByteArray::putL(uint64_t l)
{
  putl((uint32_t) (l & 0xffffffff));
  putl((uint32_t) ((l >> 32) & 0xffffffff));
}

void ByteArray::putf(float f)
{
  /*uint32_t *ptr = (uint32_t *) &f;
  putl(*ptr);*/
  uint32_t ival = *((uint32_t *) &f);
  putc(ival & 0xff);
  putc((ival >> 8) & 0xff);
  putc((ival >> 16) & 0xff);
  putc((ival >> 24) & 0xff);
}

float ByteArray::popf()
{
  uint32_t val;
  float *tmp = (float *) &val;

  uint32_t a0, a1, a2, a3;

  a0 = popc();
  a1 = popc();
  a2 = popc();
  a3 = popc();

  val = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;

  return *tmp;
}

void ByteArray::put(const void *buffer_, unsigned int len)
{
  auto buffer = (const unsigned char *) buffer_;
  for(unsigned int i = 0; i < len; i++)
    putc(buffer[i]);
}

void ByteArray::put(const ByteArray &ba)
{
  for(unsigned int i = 0; i < ba.size(); i++)
    putc(ba[i]);
}

void ByteArray::puts_zt(std::string s)
{
  for(unsigned int i = 0; i < s.size(); i++)
    putc(s[i]);
  putc(0x00);
}

void ByteArray::puts(std::string s)
{
  uint32_t n = s.size();

  if(s.size() <= 254)
  {
    putc(n);
  }
  else if(s.size() <= 65534)
  {
    putc(0xff);
    putw(n);
  }
  else
  {
    putc(0xff);
    putc(0xff);
    putc(0xff);
    putl(n);
  }


  for(unsigned int i = 0; i < n; i++)
    putc(s[i]);
}

uint32_t ByteArray::size() const
{
  return data.size();
}

uint8_t ByteArray::popc()
{
  if(size() == 0)
  {
    erreur("ByteArray::popc: size = 0.");
    return 0xff;
  }
  uint8_t res = data[0];
  data.erase(data.begin());
  return res;
}

std::string ByteArray::pops()
{
  if(size() == 0)
    return "";

  uint32_t i, n0 = popc();

  if(n0 == 0xff)
  {
    n0 = popw();
    if(n0 == 0xffff)
    {
      n0 = popl();
      if(n0 == 0xffffffff)
      {
        erreur("invalid string");
        return "";
      }
    }
  }

  char *buf = (char *) malloc(n0 + 1);

  for(i = 0; i < n0; i++)
  {
    buf[i] = popc();
  }
  buf[i] = 0;

  std::string res = std::string(buf);
  free(buf);
  return res;
}

void ByteArray::insert(uint8_t c)
{
  std::deque<unsigned char>::iterator it;
  it = data.begin();
  data.insert(it, c);
}

uint16_t ByteArray::popw()
{
  uint16_t l = popc() & 0xff;
  uint16_t b = popc() & 0xff;
  return ((b << 8) | l);
}

uint32_t ByteArray::popl()
{
  uint32_t l = popw() & 0xffff;
  uint32_t b = popw() & 0xffff;
  return ((b << 16) | l);
}

uint64_t ByteArray::popL()
{
  uint64_t l = popl() & 0xffffffff;
  uint64_t b = popl() & 0xffffffff;
  return ((b << 32) | l);
}

void ByteArray::pop_data(void *buffer_, unsigned int len)
{
  unsigned char *buffer = (unsigned char *) buffer_;
  if(len > size())
  {
    erreur("pop_data(%d), size = %d.", len, size());
    return;
  }
  for(unsigned int i = 0; i < len; i++)
  {
    buffer[i] = popc();
  }
}

void ByteArray::pop(ByteArray &ba, uint32_t len)
{
  for(uint32_t i = 0; i < len; i++)
    ba.putc(popc());
}

}
}

