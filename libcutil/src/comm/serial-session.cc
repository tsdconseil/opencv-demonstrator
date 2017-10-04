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
 *  along with LIBSERIAL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#include "comm/serial-session.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN
#ifndef VSTUDIO
#include <unistd.h>
#else
#include <process.h>
#endif
#endif




namespace utils
{

using namespace model;

namespace comm
{



#define FLAG_ACK  0x80
#define FLAG_LF   0x40
#define FLAG_RSP  0x20
#define FLAG_NCRC 0x10


//static void session_thread_entry(void *prm);

journal::Logable Transport::log("comm");
journal::Logable DataLink::log("comm");
journal::Logable Session::log("comm");

Packet::Packet()
{
  length = 0;
}

Packet::Packet(uint8_t flags, uint32_t length)
{
  this->length = length;
  this->flags = flags;
  if(length != 0)
  {
    data = (uint8_t *) malloc(length);
    if(data == nullptr)
    {
      fprintf(stderr, "Malloc failed (%d).\nAborting...\n", length);
      fflush(stderr);
      exit(-1);
    }
  }
}

Packet::Packet(uint32_t length)
{
  this->length = length;
  this->flags = 0;
  if(length != 0)
  {
    data = (uint8_t*) malloc(length);
    if(data == nullptr)
    {
      fprintf(stderr, "Malloc failed (%d).\nAborting...\n", length);
      fflush(stderr);
      exit(-1);
    }
  }
}

std::string Packet::to_string() const
{
  char bf[50];
  uint32_t i;
  std::string s = "packet flags = ";
  if(this->flags & FLAG_ACK)
    s += "ACK ";
  if(this->flags & FLAG_LF)
    s += "LF ";
  if(this->flags & FLAG_RSP)
    s += "RSP ";
  if(this->flags & FLAG_NCRC)
    s += "NCRC ";

  s += "; len = ";

  uint32_t ln = length;

  s += str::int2str(ln) + "\n";

  for(i = 0; i < ln; i++)
  {
    if((i & 15) == 0)
    {
      sprintf(bf, "%6d", i);
      s += "[" + std::string(bf) + "] ";
    }

    sprintf(bf, "%2x.", data[i]);
    s += std::string(bf);
    if((i > 0) && (((i + 1) & 15) == 0))
    {
      s += "\n";
    }
  }
  sprintf(bf, " -- total len = %d.", ln);
  s += std::string(bf);
  return s;
}

Packet::~Packet()
{
  if(length != 0)
    free(data);
}

Packet::Packet(const Packet &p)
{
  length = p.length;
  flags = p.flags;
  if(length != 0)
  {
    data = (uint8_t *) malloc(length);
    if(data == nullptr)
    {
      fprintf(stderr, "Malloc failed (%d).\nAborting...\n", length);
      exit(-1);
    }
    memcpy(data, p.data, length);
  }
}

void Packet::operator =(const Packet &p)
{
  if(length != 0)
    free(data);
  length = p.length;
  flags = p.flags;
  if(length != 0)
  {
    data = (uint8_t*) malloc(length);
    if(data == nullptr)
    {
      printf("Malloc failed (%d).\nAborting...\n", length);
      exit(-1);
    }
    memcpy(data, p.data, length);
  }
}

Packet Packet::operator +(const Packet &p)
{
  Packet res(flags, length + p.length);
  memcpy(res.data, data, length);
  memcpy(&(res.data[length]), p.data, p.length);
  return res;
}


int DataLink::start()
{
  if(!started)
  {
    hal::thread_start(this, &DataLink::com_thread, "datalink/com-thread");
    hal::thread_start(this, &DataLink::client_thread, "datalink/client-thread");
    started = true;
  }
  return 0;
}

DataLink::DataLink(IOStream *s, uint32_t max_packet_length)
: cstream(s), packets(8)
{
  log.setup("datalink");
  do_terminate = false;
  stream = s;
  started = false;
  packet_counter = 0;
  this->max_packet_length = max_packet_length;
  buffer = (uint8_t*) malloc(max_packet_length);
  if(buffer == nullptr)
  {
    erreur("Malloc failed (%d).", max_packet_length);
  }

}

DataLink::~DataLink()
{
  infos("delete.");

  if(started)
  {
    /* Tell the two threads that they must terminate */
    do_terminate = true;

    /* To unblock the first thread, if waiting for ack */
    signal_ack.raise();

    /* Push some data in the FIFO to unblock the first thread */
    Packet p;
    if(!packets.full())
      packets.push(p);

    /* Wait until first thread is finished. */
    if(signal_terminated1.wait(500))
      erreur("Unable to terminate thread 1.");

    /* Push some data in the rx queue to unblock the second thread */
    for(unsigned int i = 0; i < 1000; i++)
      cstream.putc(0xff);

    /* Wait until second thread is finished. */
    if(signal_terminated2.wait(500))
      erreur("Unable to terminate thread 2.");

    infos("Threads killed.");
  }
  free(buffer);
}

int DataLink::wait_ack(uint16_t packet_number, uint16_t timeout)
{

  for(;;)
  {
    // Attente réception acquittement
    //infos("Waiting ack %d timeout = %d ms.", packet_number, timeout);
    if(signal_ack.wait(timeout) != 0)
    {
      return -1;
    }
    if(do_terminate)
    {
      return -1;
    }

    signal_ack.clear();
    if(ack_packet_number == packet_number)
    {
      //::LeaveCriticalSection(&mutex_ack);
      mutex_ack.unlock();
      return 0;
    }
    else
    {
      //::LeaveCriticalSection(&mutex_ack);
      mutex_ack.unlock();
      erreur("Bad packet number for ack %d != %d.", ack_packet_number, packet_number);
      //continue;
      return 0;
    }
  }
  // not reached
  return -1;
}


int DataLink::put_packet(const Packet &p, uint16_t timeout)
{
/*uint32_t i;*/
  uint8_t nb_tries = 0;
  int status = -1;


  signal_ack.clear();
  mutex_put.lock();

  //infos("put_packet(flags = 0x%x, len = %d, timeout = %d).", p.flags, p.length, timeout);

  do
  {
    if(p.flags & FLAG_ACK)
    {
      erreur("Acq bit already set.");
      mutex_put.unlock();
      return -1;
    }

    nb_tries++;

    mutex_tx.lock();
    cstream.start_tx();
    cstream.putc(p.flags);
    cstream.putc(packet_counter);

    uint32_t n = p.length;

    if(n > max_packet_length)
    {
      erreur("Packet too long: %d > %d.", n, max_packet_length);
      n = max_packet_length;
    }

    //cstream.putw(n);
    // TODO: ...

    /*for(i = 0; (i < p.length) && (i < max_packet_length); i++)
      cstream.putc(p.data[i]);*/

    ByteArray ba;
    ba.putl(n);
    cstream.put(ba);

    cstream.write(p.data, n);

    uint16_t tcrc = cstream.get_current_tx_crc();
    cstream.flush();
    mutex_tx.unlock();

    //verbose("wait ack...");
    status = wait_ack(packet_counter, timeout);
    //verbose("ack = ...");


    if(status != 0)
    {
      erreur("Failed to get ACK (%d).", nb_tries);
      infos("failed packet length = %d.", p.length);
      infos("sent crc = %x.", tcrc);
      //infos("failed tx packet:\n%s\n", p.to_string().c_str());

      /*FILE *tmp = fopen("./tx_fail.txt", "wt");
      fprintf(tmp, "%s", p.to_string().c_str());
      fclose(tmp);*/
      //exit(-1);


      if(nb_tries > 2)
      {
        erreur("Aborting request.");
        break;
      }
    }
  } while(status != 0);
  packet_counter = (packet_counter + 1) % 256;
  mutex_put.unlock();
  if(status != 0)
    status = -1;
  return status;
}


void DataLink::client_thread()
{
  for(;;)
  {
    //infos("waiting new packet for client..");
    Packet p = packets.pop();

    if(do_terminate)
    {
      signal_terminated1.raise();
      infos("client thread terminated.");
      return;
    }

    //infos("new packet >> higher layer.");
    CProvider<Packet>::dispatch(p);
  }
}

void DataLink::com_thread()
{
  //infos("Com thread started.");
  for(;;)
  {
    uint8_t flags;
    int retcode;
    start:
    // Reset CRC
    cstream.start_rx();

    //trace_verbeuse("Ready...");

    retcode = cstream.getc(0);

    if(do_terminate)
    {
      infos("com thread terminated.");
      signal_terminated2.raise();
      return;
    }

    if(retcode == -1)
    {
      // (can occur during disconnection)
      infos("Timeout from lower layer");
      signal_terminated2.raise();
      return;
    }


    flags = (uint8_t) (retcode & 0xff);

    //infos("Got flags = %x.", flags);
    retcode = cstream.getc(200);
    if(retcode == -1)
    {
      erreur("cnt timeout");
      continue;
    }

    if(do_terminate)
    {
      infos("com thread terminated.");
      signal_terminated2.raise();
      return;
    }

    uint8_t pack_cnt = (uint8_t) (retcode & 0xff);

#   if 0
    //infos("Got cnt = %x.", pack_cnt);
    {
      retcode = cstream.getc(50);
      if(retcode == -1)
      {
        erreur("len timeout 1");
        continue;
      }
      r2 = cstream.getc(50);
      if(r2 == -1)
      {
        anomaly("len timeout 2");
        continue;
      }
    }
    uint16_t len = (((uint16_t) (retcode & 0xff)) << 8) | ((uint16_t) (r2 & 0xff));
#   endif


    uint32_t len;

    uint8_t tb_len[4];

    if(cstream.read(tb_len, 4, 50) != 4)
    {
      erreur("len timeout");
      continue;
    }

    ByteArray ba(tb_len, 4);
    len = ba.popl();

    if(len > this->max_packet_length)
    {
      if(do_terminate)
      {
        infos("com thread terminated.");
        signal_terminated2.raise();
        return;
      }
      else
      {
        avertissement("Length too long: %d (= 0x%x), doterm = %d",
                len, len, do_terminate);
        stream->discard_rx_buffer();
	utils::hal::sleep(20);
      }
      continue;
    }

    //verbose("Got type = 0x%x, len = %d.", flags, len);

    /* Acquittement ? */
    if(flags & FLAG_ACK)
    {
      // Check CRC
      if(cstream.check_crc() == 0)
      {
        //infos("ACK received.");
        //::EnterCriticalSection(&mutex_ack);
        ack_packet_number = pack_cnt;
        signal_ack.raise();
        //infos("rx ack %d", ack_packet_number);
      }
      else
        erreur("Bad ACK CRC.");
    }
    else
    {
      Packet p(flags, len);

      int timout = len / 2;
      if(timout < 100)
        timout = 100;

      uint32_t rlen;

      //verbose("read %d bytes..", len);
      rlen = cstream.read(p.data, len, timout);
      //verbose("done.");

      if(rlen != len)
      {
        erreur("data timeout (%d ms, %d bytes), rlen = %d.", timout, len, rlen);
        stream->discard_rx_buffer();
        goto start;
      }

      //infos("check CRC..");
      if(cstream.check_crc() == 0)
      {
        //infos("tx ack.");
        // Send ACK
        mutex_tx.lock();
        cstream.start_tx();
        cstream.putc(FLAG_ACK);
        cstream.putc(pack_cnt);
        cstream.putc(0x00);
        cstream.putc(0x00);
        cstream.putc(0x00);
        cstream.putc(0x00);
        //verbose("ack flush..");
        cstream.flush();
        //verbose("done flush.");
        mutex_tx.unlock();
        // Dispatch to higher layer
        if(packets.full())
          avertissement("Output fifo is full.");
        //verbose("Packet to fifo..");
        packets.push(p);
      }
      else
      {
        erreur("Bad data crc, len = %d.", len);
        //infos("Damaged packet:\n%s\n", p.to_string().c_str());
        FILE *tmp = fopen("./rx_fail.txt", "wt");
        fprintf(tmp, "%s", p.to_string().c_str());
        fclose(tmp);
        //exit(-1);

        stream->discard_rx_buffer();
        ComError ce;
        CProvider<ComError>::dispatch(ce);
      }
    }
  }
}


void Transport::on_event(const ComError &ce)
{
  infos("event(ComError)");
  CProvider<ComError>::dispatch(ce);
}


Transport::Transport(DataLink *stream, uint32_t tx_segmentation, uint32_t max_packet_length)
{
  log.setup("transport");
  rx_buffer_offset = 0;
  this->max_packet_length = max_packet_length;
  this->tx_segmentation = tx_segmentation;
  this->stream = stream;
  buffer = (uint8_t *) malloc(max_packet_length);
  if(buffer == nullptr)
  {
    erreur("Failed to allocate transport rx buffer (%d kbytes).", max_packet_length / 1024);
  }
  stream->CProvider<Packet>::add_listener(this);
  stream->CProvider<ComError>::add_listener(this);
}

Transport::~Transport()
{
  infos("Delete..");
  stream->CProvider<Packet>::remove_listener(this);
  stream->CProvider<ComError>::remove_listener(this);
  free(buffer);
}


int Transport::put_packet(const Packet &pin, void (*notification)(float percent))
{
  int status;
  uint32_t offset = 0;

  mutex.lock();

  //verbose("put_packet(size=%d)...", pin.length);

  status = 0;
  while(offset < pin.length)
  {
    uint8_t flags = pin.flags;

    /* Nb donn�es utiles */
    uint16_t size = tx_segmentation - 3;

    /* Last frame ? */
    if(pin.length - offset < size)
    {
      size = pin.length - offset;
      flags |= FLAG_LF;
    }

    Packet p(flags, size + 3);

    /* Offset */
    p.data[0] = (offset >> 16) & 0xff;
    p.data[1] = (offset >>  8) & 0xff;
    p.data[2] =  offset        & 0xff;
    /*for(i = 0; i < size; i++)
      p.data[3+i] = pin.data[i+offset];*/
    memcpy(&(p.data[3]), &(pin.data[offset]), size);

    //strace("write packet %d/%d: %d bytes.", offset, pin.length, size);
    status = stream->put_packet(p);
    if(status != 0)
    {
      erreur("put_packet() failed: status = %d.\n", status);
      mutex.unlock();
      return status;
    }
    offset += size;

    float percent = ((float) offset) / pin.length;
    if(notification != nullptr)
      notification(percent);
  }
  mutex.unlock();
  //infos("put_packet() successfully done\n");
  return status;
}

void Transport::on_event(const Packet &p)
{
  uint32_t i;
  uint32_t offset;

  //trace_verbeuse("got packet.");

  offset =
      ((((uint32_t) p.data[0]) << 16) & 0x00ff0000)
      | ((((uint32_t) p.data[1]) <<  8) & 0x0000ff00)
      | ((((uint32_t) p.data[2])      ) & 0x000000ff);

  if(offset < rx_buffer_offset)
  {
    erreur("offset < rx_buffer_offset (%d < %d).", offset, rx_buffer_offset);
    rx_buffer_offset = offset;
  }
  else if(offset > rx_buffer_offset)
  {
    erreur("offset > rx_buffer_offset (%d > %d).", offset, rx_buffer_offset);
    return;
  }
  else
  {
    //infos("Got offset = %d.", offset);
  }

  if(((rx_buffer_offset + p.length) - 3) > max_packet_length)
  {
    erreur("Received packet too big for receive window (mpl = %d bytes, o = %d.)",
            max_packet_length, (rx_buffer_offset + p.length) - 3);
    avertissement("Aborting reception of the packet.");
    rx_buffer_offset = 0;
    return;
  }

  rx_buffer_offset = offset;
  for(i = 3; i < p.length; i++)
    buffer[i+rx_buffer_offset-3] = p.data[i];
  rx_buffer_offset += p.length - 3;

  if(p.flags & FLAG_LF)
  {
    Packet pout(p.flags & ~FLAG_LF, rx_buffer_offset);
    memcpy(pout.data, buffer, rx_buffer_offset);
    //infos("Got last frame. size = %d bytes.", rx_buffer_offset);
    CProvider<Packet>::dispatch(pout);
    rx_buffer_offset = 0;
  }
}



Session::Session(Transport *device, uint32_t max_buffer_size):
    client_packets(8)
{
  log.setup("session");
  do_terminate = false;
  this->max_buffer_size = max_buffer_size;
  service_waited = 0xff;
  tp = device;
  session_error = false;
  if(tp != nullptr)
    tp->CProvider<Packet>::add_listener(this);
  hal::thread_start(this, &Session::client_thread, "session/client-thread");
  CListener<Packet>::listener_name = "session(packet)";
  CListener<ComError>::listener_name = "session(comError)";
}

void Session::set_transport(Transport *device)
{
  tp = device;
  tp->CProvider<Packet>::add_listener(this);
}

Session::~Session()
{
  infos("Delete..");
  tp->CProvider<Packet>::remove_listener(this);
  signal_terminated.clear();
  do_terminate = true;
  /* Wake-up client thread */
  Packet p;
  client_packets.push(p);
  signal_terminated.wait();
  infos("Thread killed.");
}

void Session::client_thread()
{
  uint32_t i, j;
  uint8_t service, cmde;
  for(;;)
  {
    //infos("client::pop...");
    Packet p  = client_packets.pop();

    if(do_terminate)
    {
      infos("client thread: terminate.");
      signal_terminated.raise();
      /* kill thread */
      return;
    }

    service   = p.data[0];
    cmde      = p.data[1];

    //trace_verbeuse("client::pop ok.");

    for(i = 0; i < cmde_handlers.size(); i++)
    {
      CmdeStorage cs = cmde_handlers[i];
      if((cs.service == service) && (cs.cmde == cmde))
      {
        ByteArray in(&(p.data[2]), p.length - 2), out;

        //verbose("Higher layer service..");
        int retcode = cs.functor->call(in, out);
        //verbose("Higher layer service done, res = %d.", retcode);

        
        if(in.size() > 0)
        {
          if(retcode == 0)
            erreur("Not all data handled by higher layer (%d bytes remaining).", in.size());
          else
            avertissement("Not all data handled by higher layer (%d bytes remaining).", in.size());
        }

        /* Send response */
        Packet p2(out.size() + 2 + 4);


        p2.flags   = FLAG_RSP;
        p2.data[0] = service;
        p2.data[1] = cmde;

        p2.data[2] = (retcode >> 24) & 0xff;
        p2.data[3] = (retcode >> 16) & 0xff;
        p2.data[4] = (retcode >> 8) & 0xff;
        p2.data[5] = (retcode ) & 0xff;

        for(j = 0; j < out.size(); j++)
          p2.data[j+2+4] = out[j];

        int res = tp->put_packet(p2, nullptr);

        if(res != 0)
        {
          erreur("Error 0x%x while putting response.", res);
          break;
        }
        break;
      } // if service ok
    } // for(i = ...)
    if(i != cmde_handlers.size())
      continue;
    else
    {
      erreur("Got unwaited data (service %x, cmde %x). Dispatching to higher layer..", service, cmde);
      dispatch(p);
    }
  } // for(;;)
}

void Session::on_event(const ComError &ce)
{
  infos("event(ComError)");
  /*if(service_waited != 0xff)
  {
  EnterCriticalSection(&response_lock);
  service_waited = 0xff;
  session_error = true;
  ::SetEvent(signal_answer);
  }*/
}

void Session::on_event(const Packet &p)
{
  //uint8_t service, cmde;

  if(p.length < 2)
  {
    erreur("Invalid packet size received: %d.", p.length);
    return;
  }


  //service = p.data[0];
  //cmde    = p.data[1];

  //infos("Rx packet: flags=0x%x, service=%x, cmde=%x, len=%d...", p.flags, service, cmde, p.length - 2);

  // Handle answers
  if(p.flags & FLAG_RSP)
  {
    if(p.length < 2 + 4)
    {
      erreur("Invalid packet size (missing result code).");
      return;
    }

    // Answer to the last request ?
    if(service_waited == p.data[0])
    {
      response_lock.lock();
      service_waited = 0xff;
      response = p;
      response.flags &= 0x0f;
      signal_answer.raise();
      //infos("Got answer.");
    }
    else
    {
      erreur("Unwaited answer: %d bytes.", p.length);
      //infos("%s", p.to_string().c_str());
    }
  }
  else
  {
    //infos(">> To client FIFO.");
    if(client_packets.full())
      avertissement("client fifo is full.");
    client_packets.push(p);
  }
}

int Session::request(uint8_t service, uint8_t cmde, 
                     const ByteArray &data_in, 
                     ByteArray &data_out, 
                     uint32_t timeout, 
                     void (*notification)(float percent))
{
  Packet p2;
  Packet p(data_in.size());
  for(unsigned int i = 0; i < data_in.size(); i++)
    p.data[i] = data_in[i];
  int res = request(service, cmde, p, p2, timeout, notification);
  if(res == 0)
    data_out = ByteArray(p2.data, p2.length);
  else
  {
    avertissement("Request failed, status = %d.", res);
    data_out.clear();
  }
  return res;
}

int Session::request(uint8_t service, uint8_t cmde, const ByteArray &data_in, uint32_t timeout)
{
  ByteArray out;
  return request(service, cmde, data_in, out, timeout, nullptr);
}

int Session::request(uint8_t service,
                     uint8_t cmde, 
                     const Packet &data_in, Packet &data_out,
                     uint32_t timeout,
                     void (*notification)(float percent))
{
  uint32_t i;
  int res;

  request_lock.lock();
  //infos("request(service=0x%x, cmde 0x%x, tx_len = %d, timeout = %d).", service, cmde, data_in.length, timeout);

  service_waited = service;

  //p.flags |= FLAG_REQUEST;

  Packet p(data_in.length + 2);

  p.data[0] = service;
  p.data[1] = cmde;

  for(i = 0; i < data_in.length; i++)
    p.data[i+2] = data_in.data[i];
  res = tp->put_packet(p, notification);

  if(res != 0)
  {
    request_lock.unlock();
    service_waited = 0xff;
    erreur("Error 0x%x while putting request.", res);
    return res;
  }

  //infos("Put done, now waiting read answer...");
  if(signal_answer.wait(timeout) == 0)
  {
    service_waited = 0xff;
    signal_answer.clear();
    request_lock.unlock();
    /* Remove service_id, cmde_id and status */
    data_out = Packet(response.length - 6);
    int status =   (((int) response.data[2] << 24) & 0xff000000)
                 | (((int) response.data[3] << 16) & 0x00ff0000)
                 | (((int) response.data[4] <<  8) & 0x0000ff00)
                 | (((int) response.data[5]) & 0x000000ff);

    for(i = 0; i < response.length - 6; i++)
      data_out.data[i] = response.data[i+6];

    response_lock.unlock();

    if(session_error)
    {
      service_waited = 0xff;
      session_error = false;
      erreur("Session error.");
      return -1;
    }

    //infos("Read: status = %d.", status);
    return status;
  }
  else
  {
    service_waited = 0xff;
    erreur("No response.");
    // timeout
    request_lock.unlock();
    return -1;
  }
}

int Session::register_cmde(uint8_t service, uint8_t cmde, CmdeFunctor *functor)
{
  CmdeStorage storage;
  storage.service = service;
  storage.cmde    = cmde;
  storage.functor = functor;
  cmde_handlers.push_back(storage);
  return 0;
}

}
}
