#ifndef IOSTREAMS_HPP
#define IOSTREAMS_HPP

/**
 *  This file is part of LIBCUTIL.
 *
 *  LIBCUTIL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBCUTIL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#include "bytearray.hpp"

//#include <stdio.h>
#include <string>


#ifdef putc
#undef putc
#endif

namespace utils
{
/** @brief Protocols and communication interfaces */
namespace comm
{



/** @brief Abstract input stream interface */
class InputStream
{
public:
  virtual int getc(int timeout = 0) = 0;
  unsigned short getw();
  void  get_data(char *buffer, int len);

  /** @brief Read a line of (text) data from the stream.
   *  The line must be terminated by '\n'
   *  @param[out] res The line of text readen, '\n' caracter not included.
   *  @param timeout Timeout in milliseconds;
   *  @returns -1 if timeout occured */
  int get_line(std::string &res, int timeout);

  /** @return number of bytes read */
  virtual int read(uint8_t *buffer, uint32_t length, int timeout);

  int read(model::ByteArray &ba, uint32_t length, int timeout);

  virtual void discard_rx_buffer();
protected:
private:
};

/** @brief Abstract output stream interface */
class OutputStream
{
public:
  virtual void write(const uint8_t *buffer, uint32_t len);
  virtual void putc(char c) = 0;
  void put(const model::ByteArray &ba);
  void put_data(const char *buffer, int len);
  void put_string(std::string s);
  void putw(unsigned short s);
  virtual void flush(){}
protected:
private:
};

class IOStream : public InputStream, public OutputStream
{
};

class CRCStream: public IOStream
{
public:
  CRCStream(IOStream *stream);

  virtual void putc(char c);
  void start_tx();
  void flush();
  void write(const uint8_t *buffer, uint32_t len);

  virtual int read(uint8_t *buffer, uint32_t length, int timeout);
  virtual int getc(int timeout);
  void start_rx();
  int check_crc();

  uint16_t get_current_tx_crc();

private:
  IOStream *stream;
  uint16_t current_tx_crc;
  uint16_t current_rx_crc;
};

}
}

#endif

