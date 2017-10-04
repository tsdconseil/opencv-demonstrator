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
#include "cutil.hpp"

#include <stdio.h>
#include <assert.h>

#ifdef WIN
#ifndef VSTUDIO
#include <unistd.h>
#else
#include <process.h>
#endif
#include <windows.h>
#else

#endif
#include <sys/types.h>
#include <malloc.h>

#define DBG(aaa)


namespace utils
{
namespace comm
{

FDSerial::FDSerial()
{
  serial_is_connected = false;
  input_buffer = (char*) malloc(FD_BUFFER_SIZE);
  output_buffer = (char*) malloc(FD_BUFFER_SIZE);
  assert(input_buffer);
  assert(output_buffer);
  hal::thread_start(this, &FDSerial::com_thread, "fdserial/com-thread");
}

FDSerial::~FDSerial()
{
  disconnect();
}

void FDSerial::putc(char c)
{
  if(!serial_is_connected)
  {
    erreur("putc while not connected.");
    return;
  }

  while(output_buffer_size >= FD_BUFFER_SIZE-1)
    hevt_tx_done.wait();

  mutex_output.lock();
  output_buffer[(output_buffer_offset+output_buffer_size)%FD_BUFFER_SIZE] = c;
  output_buffer_size++;
  mutex_output.unlock();
  hevt_tx_available.raise();
}

void FDSerial::flush()
{
  for(;;)
  {
    mutex_output.lock();
    if(output_buffer_size == 0)
    {
      printf("Flush : obs = 0.\n");
      mutex_output.unlock();
      return;
    }
    mutex_output.unlock();
    printf("flush / tx size = %d.\n", output_buffer_size);
    hevt_tx_done.wait();
    hevt_tx_done.clear();
  }
  //fflush(hfile);
  //FlushFileBuffers(hfile);
}

unsigned int FDSerial::nb_rx_available()
{
  return input_buffer_size;
}

void FDSerial::discard_rx_buffer()
{
  infos("Vidange de la FIFO de réception...");
  mutex_input.lock();

  if(input_buffer_size > 0)
  {
    input_buffer_offset = (input_buffer_offset + input_buffer_size) % FD_BUFFER_SIZE;
    input_buffer_size = 0;
  }

  /*while(input_buffer_size > 0)
  {
    input_buffer_offset = (input_buffer_offset + input_buffer_size) % FD_BUFFER_SIZE;
    input_buffer_size = 0;
    mutex_input.unlock();
    hal::sleep(100);
    mutex_input.lock();
  }*/
  mutex_input.unlock();
  infos("Ok.");
}

void FDSerial::debloquer_reception()
{
  trace_verbeuse("FDSerial::%s", __func__);
  signal_debloquer_reception.raise();
}

int FDSerial::getc(int timeout)
{
  if(!serial_is_connected)
  {
    infos("Read suspended until serial port is opened.");
    while(!serial_is_connected)
    {
      hevt_connection.wait();
      hevt_connection.clear();
      infos("+");
    }
    infos("Read enabled.");
  }

  std::vector<utils::hal::Signal *> sigs;
  sigs.push_back(&hevt_rx_available);
  sigs.push_back(&signal_debloquer_reception);

  mutex_input.lock();
  while(input_buffer_size == 0)
  {
    mutex_input.unlock();

    //utils::verbose("GETC WAIT %d ms...", timeout);
    int res = utils::hal::Signal::wait_multiple(timeout, sigs);
    //utils::verbose("GETC RES = %d.", res);

    if((res == 1) || (signal_debloquer_reception.is_raised()))
    {
      trace_verbeuse("FDSerial::getc: reception annulee sur ordre.");
      return -1;
    }

    if(res != 0)
      return -1;

    //if(hevt_rx_available.wait(timeout))
    // return -1;

    mutex_input.lock();
  }
  char c = input_buffer[input_buffer_offset];

  input_buffer_offset = (input_buffer_offset + 1) % FD_BUFFER_SIZE;
  input_buffer_size--;
  mutex_input.unlock();
  return (((int) c) & 0xff);
}  






int  FDSerial::connect(std::string port_name,
                       int baudrate,
                       serial_parity_t parity,
                       bool flow_control)
{
# ifdef LINUX
  return -1;
# else
  try
  {
    infos("Connection %s @ %d bauds, ctrl de flux = %s...",
              port_name.c_str(), baudrate, flow_control ? "oui" : "non");

    char port[50];

    sprintf(port, "%s", port_name.c_str());
    if(strlen(port) > 4)
    {
      sprintf(port, "%s%s", "\\\\.\\", port_name.c_str());
      infos("Added prefix.");
    }

    if(serial_is_connected)
    {
      erreur("Already connected.");
      return 0;
    }

#   ifdef VSTUDIO
    wchar_t temp[100];
    mbstowcs(temp, port, 100);
#   endif

    hfile = ::CreateFile(
#ifdef VSTUDIO
        temp,
#else
        port,
#endif
        GENERIC_READ|GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        0);


    if(hfile == nullptr)
    {
      erreur("CreateFile error.");
      return -1;
    }

    if(hfile == INVALID_HANDLE_VALUE)
    {
      int err = ::GetLastError();
      avertissement("Erreur CreateFile (erreur = %d = 0x%x).", err, err);
      return -1;
    }

    infos("CreateFile ok.");

    if(!::SetCommMask(hfile, EV_RXCHAR))
    {
      int err = ::GetLastError();
      avertissement("Erreur SetCommMask (erreur = %d = 0x%x).", err, err);
      return -1;
    }

    infos("Set Comm mask ok.");

    COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };

    if(!SetCommTimeouts(hfile, &cto))
    {
      int err = ::GetLastError();
      avertissement("Unable to set comm timeouts (erreur = %d = 0x%x).", err, err);
      return -1;
    }

    infos("Set Comm timeouts ok.");

    DCB dcb;
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
    dcb.fDtrControl     = flow_control ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = 0;
    dcb.fRtsControl     = RTS_CONTROL_ENABLE;
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

    // set DCB
    if(!SetCommState(hfile,&dcb))
    {
      erreur("Error while setting comm state.");
      CloseHandle(hfile);
      return -1;
    }

    infos("Set DCB ok.");

    HANDLE	h_evt_overlapped;
    h_evt_overlapped = ::CreateEvent(0,true,false,0);

    memset(&ov,0,sizeof(ov));
    ov.hEvent = h_evt_overlapped;
    assert(ov.hEvent);



    input_buffer_offset = 0;
    input_buffer_size = 0;
    output_buffer_offset = 0;
    output_buffer_size = 0;

    hevt_start.raise();
    serial_is_connected = true;
    hevt_connection.raise();
  }
  catch(...)
  {
    return -1;
  }
  return 0;
# endif
}

bool FDSerial::is_connected()
{
  return serial_is_connected;
}



void FDSerial::disconnect()
{
  if(serial_is_connected)
  {
    infos("Arret thread de reception fdserial...");
    hevt_stop.raise();
    hevt_stopped.wait();
    infos("Thread arrete.");
#   ifdef LINUX
#   else
    infos("Vidange des tampons rx/tx...");
    ::FlushFileBuffers(hfile);
    infos("Fermeture handle...");
    CloseHandle(hfile);
#   endif
    serial_is_connected = false;
    infos("Deconnexion terminee.");
  }
}

void FDSerial::com_thread(void)
{
  infos("Com thread started.");

  hevt_start.wait();

  infos("Com thread resumed.");

# ifdef LINUX
# else
  bool is_reading;
  for(;;)
  {

    start:
    is_reading = true;
    // par d�faut: lance une lecture
    char c;
    DWORD nb_read;
    if (!::ReadFile(hfile,&c,1,&nb_read,&ov))
    {
      DBG(printf("Read deffered.\n"));
      DBG(fflush(stdout));
    }
    else
    {
      DBG(printf("Read succeedded immediatly : '%x'!!!\n", (unsigned char) c));
      DBG(fflush(stdout));

      mutex_input.lock();
      if(input_buffer_size >= FD_BUFFER_SIZE)
      {
        erreur("Input buffer overflow.");
      }
      else
      {
        input_buffer[(input_buffer_offset+input_buffer_size)%FD_BUFFER_SIZE] = c;
        input_buffer_size++;
      }
      mutex_input.unlock();
      hevt_rx_available.raise();
      goto start;
    }


    HANDLE ahWait[3];
    ahWait[0] = ov.hEvent;
    ahWait[1] = hevt_stop.get_handle();
    ahWait[2] = hevt_tx_available.get_handle();


    wait_event:
    switch (::WaitForMultipleObjects(sizeof(ahWait)/sizeof(*ahWait),ahWait,FALSE,INFINITE))
    {
    case WAIT_OBJECT_0:
    {
      if(is_reading)
      {
        DBG(printf("Evt overlapped, on peut faire maintenant un vrai 'read'...\n");
        fflush(stdout);)
        DWORD nb_readen;
        if(!::GetOverlappedResult(hfile,&ov,&nb_readen,FALSE))
          erreur("Error %d\n", GetLastError());
        else
        {
          if(nb_readen > 0)
          {
            mutex_input.lock();
            if(input_buffer_size >= FD_BUFFER_SIZE)
            {
              erreur("Input buffer overflow.");
            }
            else
            {
              input_buffer[(input_buffer_offset+input_buffer_size)%FD_BUFFER_SIZE] = c;
              input_buffer_size++;
              if(nb_readen > 1)
              {
                erreur("nb readen = %d.", nb_readen);
              }
            }
            mutex_input.unlock();
            hevt_rx_available.raise();
            DBG(printf("Read %d bytes succeeded : '%x'.\n", nb_readen, (unsigned char) c));
            DBG(fflush(stdout));
          }
        }
      }
      else
      {
        DWORD nb_wrote;
        if(!::GetOverlappedResult(hfile,&ov,&nb_wrote,FALSE))
        {
          erreur("Error write %d\n", GetLastError());
        }
        else
        {
          bool relance = false;
          // Termin� une �criture, on en lance �ventuellement une autre
          mutex_output.lock();
          if(output_buffer_size > 0)
            relance = true;
          mutex_output.unlock();
          DBG(printf("Evt overlapped, le write %d bytes est termin�...\n", nb_wrote));
          DBG(fflush(stdout));
          if(relance)
          {
            DBG(printf("On relance une �criture...\n"));
            DBG(fflush(stdout));
            hevt_tx_available.raise();
          }
          else
          {
            hevt_tx_done.raise();
            //::SetEvent(hevt_tx_done);
          }
        }
        DBG(fflush(stdout));
      }
      break;
    }
    case WAIT_OBJECT_0+1:
  {
    hevt_stop.clear();
    infos("Recu stop: PurgeComm...");
    ::PurgeComm(hfile, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    //assert(CancelIo(hfile));
    //hal::sleep(100);
    infos("Cancel io...");
    CancelIo(hfile);
    hevt_stopped.raise();

    infos("Serial port stopped.");

    hevt_start.wait();
    infos("Received Start");

    break;
  }
    case WAIT_OBJECT_0+2:
  {
    DBG(printf("Evt write\n"));
    DBG(fflush(stdout));
    hevt_tx_available.clear();

    if(is_reading)
    {
      DWORD n = 0;


      // Attention perte d'un octet en lecture
      ////////assert(CancelIo(hfile));

      /////////////////////////
      /// V�rifie rien � lire sur l'entr�e avant d'annuler l'�criture
      DWORD nb_readen;
      if(::GetOverlappedResult(hfile,&ov,&nb_readen,FALSE))
      {
        if(nb_readen > 0)
        {
          mutex_input.lock();
          if(input_buffer_size >= FD_BUFFER_SIZE)
            erreur("Input buffer overflow.");
          else
          {
            input_buffer[(input_buffer_offset+input_buffer_size)%FD_BUFFER_SIZE] = c;
            input_buffer_size++;
            if(nb_readen > 1)
              erreur("nb readen = %d.", nb_readen);
          }
          mutex_input.unlock();
          hevt_rx_available.raise();
        }
      }
      else
        assert(CancelIo(hfile));

      /////////////////////////






      mutex_output.lock();
      // Ecriture en deux temps ?
      unsigned long lg = output_buffer_size;
      if(output_buffer_offset + output_buffer_size > FD_BUFFER_SIZE)
        lg = FD_BUFFER_SIZE - output_buffer_offset;

      if(!WriteFile(hfile, &(output_buffer[output_buffer_offset]), lg, &n, &ov))
      {
        DBG(printf("Write deffered.\n"));
        DBG(fflush(stdout));
        is_reading = false;
        output_buffer_offset = (output_buffer_offset + lg) % FD_BUFFER_SIZE;
        output_buffer_size -= lg;
        mutex_output.unlock();

        if(output_buffer_size == 0)
          hevt_tx_done.raise();

        goto wait_event;
      }
      else
      {
        output_buffer_offset = (output_buffer_offset + lg) % FD_BUFFER_SIZE;
        output_buffer_size -= lg;
        if(output_buffer_size == 0)
          hevt_tx_done.raise();
         //::SetEvent(hevt_tx_done);
        mutex_output.unlock();
        DBG(printf("Write finished immediatly.\n"));
        DBG(fflush(stdout));
      }
    }
    // On est d�j� en train d'�crire: on fait rien
    else
    {
      DBG(printf("Write alors qu'on �crit d�j�.\n"));
      DBG(fflush(stdout));
      goto wait_event;
    }
    break;
  }
    default:
    {
      erreur("Evt inconnu");
      break;
    }
    }
  }
# endif
}

}
}



