/**
 *  This file is part of LIBSERIAL.
 *
 *  LIBSERIAL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBSERIAL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBSERIAL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "comm/iostreams.hpp"
#include "trace.hpp"
#include "cutil.hpp"

#include <stdint.h>
#include <stdio.h>
#ifdef WIN
#include <windows.h>
#endif
#include <string>


namespace utils
{
namespace comm
{

using namespace utils;

class EscapedIOStream : public IOStream
{
public:
  EscapedIOStream(IOStream *c);
  void putc(char c);
  int getc(int timeout = 0);
  void start_frame();
  void end_frame();
  int get_frame(char *buffer, unsigned int max_len);
  virtual void discard_rx_buffer(){compo->discard_rx_buffer();}
private:
  IOStream *compo;
  unsigned short tx_crc;
};

class SerialInfo
{
public:
  std::string name;
  std::string complete_name;
  std::string techno;
};


typedef enum serial_parity_enum  
{ 
  PAR_NONE,
  PAR_ODD, 
  PAR_EVEN
} serial_parity_t;

class Serial : public IOStream
{
public:
  static int enumerate(std::vector<SerialInfo> &infos);
  Serial();
  virtual ~Serial();
  /** @returns 0 si ok, sinon code d'erreur */
  int  connect(std::string port_name = "COM1", int baudrate = 115200, 
	       serial_parity_t parity = PAR_NONE);

  virtual void putc(char c);
  virtual int getc(int timeout = 0);
  virtual void discard_rx_buffer(){}

  int  get_nb_bytes_available();
  void disconnect();
  bool is_connected();
  void set_timeout(int tm);
protected:
  std::string port_name;
  int baudrate;
  serial_parity_t parity;
# ifdef WIN
  HANDLE serial_handle;
# endif
  bool connected;
};

#define FD_BUFFER_SIZE (128*1024)

class FDSerial : public IOStream
{
public:
  FDSerial();
  virtual ~FDSerial();
  virtual void putc(char c);
  virtual int getc(int timeout = 0);
  //virtual int read(uint8_t *buffer, uint32_t length, int timeout);
    /** @returns 0 si ok, sinon code d'erreur */
  int  connect(std::string port_name = "COM2", int baudrate = 115200, 
	       serial_parity_t parity = PAR_NONE);
  void disconnect();
  bool is_connected();
  virtual void discard_rx_buffer();
  unsigned int nb_rx_available();
private:
  
  void com_thread(void);

  hal::Mutex mutex_input, mutex_output;
  hal::Signal hevt_stop;
  hal::Signal hevt_write, hevt_tx_done;
  hal::Signal hevt_rx_available;
  hal::Signal hevt_tx_available, hevt_start, hevt_stopped, hevt_connection;
# ifdef WIN
  OVERLAPPED ov;
  HANDLE hfile;
# endif
  char *input_buffer;
  uint32_t input_buffer_offset, input_buffer_size;
  char *output_buffer;
  uint32_t output_buffer_offset, output_buffer_size;
  bool serial_is_connected;
  uint32_t nb_bytes_waited;
  Logable log;
};



class SerialConfig
{
public:
  SerialConfig(){port="COM1";baud_rate=115200;}
  std::string port;
  int baud_rate;
  void dump();
};



}
}




#endif

