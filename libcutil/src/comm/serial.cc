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

#include "comm/serial.hpp"
#include "comm/crc.hpp"

//#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN
#include <process.h>
#include <conio.h>
#include <windows.h>
#endif



namespace utils
{
namespace comm
{

//#define serial_trace(...) 


#ifndef serial_trace
static void serial_trace(const char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  printf("\033[32m[serial] \033[0m");
  vprintf(s, ap);
  printf("\r\n");
  fflush(stdout);
  va_end(ap);
}
#endif





EscapedIOStream::EscapedIOStream(IOStream *c)
{
  compo = c;
}

void EscapedIOStream::start_frame()
{
  compo->putc(0xff);
  tx_crc = 0xffff;
}

void EscapedIOStream::end_frame()
{
  // Write CRC
  unsigned short the_crc = tx_crc;
  putc(the_crc & 0xff);
  putc((the_crc >> 8) & 0xff);
  compo->putc(0xfe);
}

void EscapedIOStream::putc(char c)
{
  unsigned char cc = (unsigned char) c;

  // update crc
  tx_crc = crc_update(tx_crc, c);

  if(cc == 0xff)
  {
    compo->putc(0xfd);
    compo->putc(0x02);
  }
  else if(cc == 0xfe)
  {
    compo->putc(0xfd);
    compo->putc(0x01);
  }
  else if(cc == 0xfd)
  {
    compo->putc(0xfd);
    compo->putc(0x00);
  }
  else
    compo->putc(c);
}

int EscapedIOStream::getc(int timeout)
{
  int c = compo->getc(timeout);
  return c;
}

static void print_frame(char *buffer, unsigned int len)
{
  printf("Frame(%d): ", len);
  for(unsigned int i = 0; i < len; i++)
    printf("%x.", (unsigned char) buffer[i]);
  printf("\n");
  fflush(stdout);
}

int EscapedIOStream::get_frame(char *buffer, unsigned int max_len)
{
  unsigned int frame_size;
  unsigned char c;

  //wait_sof:
  do
  {
    c = getc();
  } while(c != 0xff);

  //sof:
  frame_size = 0;
  for(;;)
  {
    c = getc();
    

    if((unsigned char) c == 0xff)
    {
      printf("EIOStream::unexpected sof\n");
      return -1;
      //goto sof;
    }
    if((unsigned char) c == 0xfe)
      break;

    if((unsigned char) c == 0xfd)
    {
      c = getc() + 0xfd;
    }

    if(frame_size == max_len)
    {
      printf("EIOStream::frame too large (> %d)\n", max_len);
      fflush(stdout);
      return -1;
      //goto wait_sof;
    }
    buffer[frame_size++] = c;
  }
  if(frame_size < 2)
  {
    printf("Frame too short.\n");
    fflush(stdout);
    return -1;
    //goto wait_sof;
  }
  unsigned short crc = 0xffff;
  unsigned short i;
  for(i = 0; i < frame_size - 2; i++)
    crc = crc_update(crc, buffer[i]);
  if((unsigned char) (crc & 0xff) != (unsigned char) buffer[frame_size-2])
  {
    printf("*******************************\nBad CRC (lsb)\n");
    printf("Expected: %x, received: %x%x.\n", crc, (unsigned char) buffer[frame_size-2], (unsigned char) buffer[frame_size-1]);
    print_frame(buffer, frame_size);
    fflush(stdout);
    return -1;
    //goto wait_sof;
  }
  if((unsigned char) ((crc>>8)&0xff) != (unsigned char) buffer[frame_size-1])
  {
    printf("Bad CRC (msb)\n");
    printf("Expected: %x, received: %x%x.\n", crc, (unsigned char) buffer[frame_size-2], (unsigned char) buffer[frame_size-1]);

    //print_frame(buffer, frame_size);
    fflush(stdout);
    return -1;
    //goto wait_sof;
  }

  //printf("GOOD FRAME:\n");
  //print_frame(buffer, frame_size);
  //fflush(stdout);

  // - 2 : CRC
  return frame_size - 2;
}


bool Serial::is_connected()
{
  return connected;
}

Serial::Serial()
{
  parity = PAR_NONE;
  port_name = "";
  baudrate = 0;
# ifdef WIN
  serial_handle = INVALID_HANDLE_VALUE;
# endif
  connected = false;
}

Serial::~Serial()
{
# ifdef WIN
  if (serial_handle!=INVALID_HANDLE_VALUE)
    CloseHandle(serial_handle);
  serial_handle = INVALID_HANDLE_VALUE;
# endif
}

void Serial::disconnect(void)
{
# ifdef WIN
  if (serial_handle!=INVALID_HANDLE_VALUE)
    CloseHandle(serial_handle);
  serial_handle = INVALID_HANDLE_VALUE;
# endif
}

void Serial::set_timeout(int tm)
{
# ifdef WIN
  COMMTIMEOUTS cto = { tm, 0, 0, 0, 0 };
  if(serial_handle != INVALID_HANDLE_VALUE)
  {
    if(!SetCommTimeouts(serial_handle,&cto))
      ;
  }
# endif
}


int  Serial::connect(std::string port_name, int baudrate, 
		     serial_parity_t parity)
{
# ifdef WIN
  int erreur;
  DCB  dcb;
  COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };
  
  if (serial_handle!=INVALID_HANDLE_VALUE)
    CloseHandle(serial_handle);
  serial_handle = INVALID_HANDLE_VALUE;
  
  erreur = 0;

  this->port_name = port_name;
  this->baudrate = baudrate;
  this->parity = parity;
  memset(&dcb,0,sizeof(dcb));

  
  // set DCB to configure the serial port
  dcb.DCBlength       = sizeof(dcb);                   
        
  dcb.BaudRate        = baudrate;

  switch(parity)
    {
    case PAR_NONE:
      dcb.Parity      = NOPARITY;
      dcb.fParity     = 0;
      break;
    case PAR_EVEN:
      dcb.Parity      = EVENPARITY;
      dcb.fParity     = 1;
      break;
    case PAR_ODD:
      dcb.Parity      = ODDPARITY;
      dcb.fParity     = 1;
      break;
    }


  dcb.StopBits        = ONESTOPBIT;
  dcb.ByteSize        = 8;
        
  dcb.fOutxCtsFlow    = 0;
  dcb.fOutxDsrFlow    = 0;
  dcb.fDtrControl     = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fRtsControl     = RTS_CONTROL_DISABLE;
  dcb.fOutX           = 0;
  dcb.fInX            = 0;
        
  /* ----------------- misc parameters ----- */
  dcb.fErrorChar      = 0;
  dcb.fBinary         = 1;
  dcb.fNull           = 0;
  dcb.fAbortOnError   = 0;
  dcb.wReserved       = 0;
  dcb.XonLim          = 2;
  dcb.XoffLim         = 4;
  dcb.XonChar         = 0x13;
  dcb.XoffChar        = 0x19;
  dcb.EvtChar         = 0;
        
  serial_handle    = ::CreateFile(
#ifdef VSTUDIO
    (LPCWSTR)
#endif
    port_name.c_str(), 
				GENERIC_READ | GENERIC_WRITE,
				0, 
				  nullptr, OPEN_EXISTING,/*nullptr*/0,nullptr);

  if(serial_handle != INVALID_HANDLE_VALUE)
  {
    if(!SetCommMask(serial_handle, 0))
      erreur = 1;

    // set timeouts
    if(!SetCommTimeouts(serial_handle,&cto))
      erreur = 2;
    
    // set DCB
    if(!SetCommState(serial_handle,&dcb))
      erreur = 4;
  }
  else
    erreur = 8;
  
  if (erreur!=0)
  {
    CloseHandle(serial_handle);
    serial_handle = INVALID_HANDLE_VALUE;
    connected = false;
  }
  else
    connected = true;
  return(erreur);
# else
  return -1;
# endif
}


void Serial::putc(char data)
{
# ifndef LINUX
  unsigned long result;

  if (serial_handle!=INVALID_HANDLE_VALUE)
    WriteFile(serial_handle, &data, 1, &result, nullptr);
# endif
}






int Serial::getc(int timeout)
{
  char c = 0;
# ifdef WIN
  if(serial_handle == INVALID_HANDLE_VALUE)
    return 0;

  unsigned long read_nbr = 0;

  ReadFile(serial_handle, &c, 1, &read_nbr, nullptr);
# endif

  return c;
}



int Serial::get_nb_bytes_available(void)
{
# ifdef WIN
  struct _COMSTAT status;
  int             n;
  unsigned long   etat;

  n = 0;
  
  if (serial_handle!=INVALID_HANDLE_VALUE)
  {
    ClearCommError(serial_handle, &etat, &status);
    n = status.cbInQue;
  }
  return n;
# else
  return 0;
# endif
}



CRCStream::CRCStream(IOStream *stream)
{
  this->stream = stream;
  current_tx_crc = 0;
  current_rx_crc = 0;
}

void CRCStream::putc(char c)
{
  // update crc
  current_tx_crc = crc_update(current_tx_crc, c);
  stream->putc(c);
}

void CRCStream::write(const uint8_t *buffer, uint32_t len)
{
  if(len > 0)
  {
    uint32_t i;
    for(i = 0; i < (uint32_t) len; i++)
      current_tx_crc = crc_update(current_tx_crc, buffer[i]);
  }
  stream->write(buffer, len);
}

void CRCStream::start_tx()
{
  current_tx_crc = 0xffff;
}

void CRCStream::flush()
{
  stream->putc((current_tx_crc >> 8) & 0xff);
  stream->putc(current_tx_crc & 0xff);
  stream->flush();
}

uint16_t CRCStream::get_current_tx_crc()
{
  return current_tx_crc;
}

int CRCStream::read(uint8_t *buffer, uint32_t length, int timeout)
{
  int res = stream->read(buffer, length, timeout);
  if(res > 0)
  {
    uint32_t i;
    //printf("compute crc %d bytes..\n", length);
    for(i = 0; i < (uint32_t) res; i++)
      current_rx_crc = crc_update(current_rx_crc, buffer[i]);
    //printf("done.\n");
  }

  return res;
}

int  CRCStream::getc(int timeout)
{
  int res = stream->getc(timeout);
  if(res != -1)
    current_rx_crc = crc_update(current_rx_crc, (unsigned char) (res & 0xff));
  return res;
}

void CRCStream::start_rx()
{
  current_rx_crc = 0xffff;
}

int  CRCStream::check_crc()
{
  unsigned short c1, c2;
  int r;
  
  r = stream->getc(250);
  if(r == -1)
  {
    serial_trace("Error while reading crc (1).");
    return -1;
  }
  c1 = r & 0xff;
  c1 = c1 << 8;
  r = stream->getc(250);
  if(r == -1)
  {
    serial_trace("Error while reading crc (2).");
    return -1;
  }

  c2 = r & 0xff;
  c2 = c2 | c1;

  if(c2 != current_rx_crc)
  {
    serial_trace("bad crc : computed = %x, got = %x.", current_rx_crc, c2);
    return -1;
  }
  return 0; 
}

}
}


