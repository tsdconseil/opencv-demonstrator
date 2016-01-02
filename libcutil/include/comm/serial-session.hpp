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

#ifndef SERIAL_SESSION_HPP
#define SERIAL_SESSION_HPP

#include "comm/iostreams.hpp"
#include "cutil.hpp"
#include "slots.hpp"
#include "trace.hpp"
#include "bytearray.hpp"

#include <stdint.h>
#include <queue>

namespace utils
{
namespace comm
{

using namespace utils;

// 500 ms seems to be too small, at least when debugging
#define COMM_DEFAULT_TIMEOUT            500
#define COMM_SESSION_DEFAULT_TIMEOUT    800

/** @brief Data packet container */
class Packet
{
public:
  Packet();
  Packet(uint8_t flags, uint32_t length);
  Packet(uint32_t length);
  ~Packet();
  Packet(const Packet &p);
  void operator =(const Packet &p);
  Packet operator +(const Packet &p);
  std::string to_string() const;

  /** @brief Data buffer pointer */
  uint8_t *data;

  /** @brief Data buffer length */
  uint32_t length;

  /** @brief Packet flags (8 bits) */
  uint8_t  flags;
};

/** @brief Dummy class to notify communication errors */
class ComError{};

/** @brief Enable small packets (<= 4 ko) exchange with:
 *         - CRC check
 *         - ACQ management / retries on failure, with packet counter.
 *  Un thread de plus (s�pa ack VS donn�es in) */
class DataLink: 
  public  CProvider<Packet>,  /* Provides received packets to higher layer */
  public  CProvider<ComError> /* Dispatch error notifications to higher layer */
{
public:
  DataLink(IOStream *s, uint32_t max_packet_length = 2*8192);
  ~DataLink();
  int put_packet(const Packet &pkt, uint16_t timeout = COMM_DEFAULT_TIMEOUT);
  /** Start listening for input packets on the iostream */
  int start();
private:
  /** Already started listening on the iostream ? */
  bool started;
  void com_thread();
  void client_thread();
  int wait_ack(uint16_t packet_number, uint16_t timeout);
  uint16_t ack_packet_number;
  uint32_t max_packet_length;
  IOStream *stream;
  CRCStream cstream;
  hal::Mutex mutex_ack, mutex_put;
  hal::Signal signal_ack;
  uint8_t *buffer;
  uint16_t packet_counter;
  static Logable log;
  
  bool do_terminate;
  hal::Signal signal_terminated1, signal_terminated2;

  hal::Fifo<Packet> packets;

  enum dl_type_enum
  {
    DL_ACK = 0,
    DL_NACK = 1,
    DL_DATA = 2
  };

  hal::Mutex mutex_tx;
};


/** @brief Long packets segmentation. */
class Transport: 
  public  CProvider<Packet>,  /* Provides received packets to higher layer */
  public  CProvider<ComError>,/* Dispatch error notifications to higher layer */
  private CListener<Packet>,  /* Read packets from lower layer */
  private CListener<ComError> /* Read error notifications from lower layer */

{
public:
  Transport(DataLink *stream,
            /** Max length of PDU packets in emission */
            uint32_t tx_segmentation = 256,
            /** Max length of reassembled packets in reception */
            uint32_t max_packet_length = 1024*1024*4/*32*/);
  virtual ~Transport();
  int put_packet(const Packet &p, void (*notification)(float percent) = nullptr);
  void on_event(const Packet &p);
  void on_event(const ComError &ce);
private:
  DataLink *stream;
  uint32_t rx_buffer_offset, rx_buffer_length;
  uint32_t max_packet_length, tx_segmentation;
  uint8_t *buffer;
  hal::Mutex mutex;
  static Logable log;
};


class CmdeFunctor
{
public:
  virtual int call(model::ByteArray &data_in, model::ByteArray &data_out) = 0;
};

template <class A>
class SpecificCmdeFunctor: public CmdeFunctor
{
public:
  SpecificCmdeFunctor(A *object, int(A::*m_function)(model::ByteArray &data_in, model::ByteArray &data_out))
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual int call(model::ByteArray &data_in, model::ByteArray &data_out)
  {
    return (*object.*m_function)(data_in, data_out);
  }
private:
  int (A::*m_function)(model::ByteArray &, model::ByteArray &);
  A *object;
};


/** @brief Une session, notions de :
 *  - Requ�te / r�ponse
 *  - n� de service
*/
class Session: 
  public  CProvider<Packet>, /* Provides request packets to higher layer */
  private CListener<Packet>, /* Read packets from lower layer */
  private CListener<ComError>/* Read error notifications from lower layer */
{
public:

  Session(Transport *device = nullptr, uint32_t max_buffer_size = 65536);
  void set_transport(Transport *device);

  ~Session();

  /** Transmit a read request to the remote pair */
  int request(uint8_t service, uint8_t cmde, const model::ByteArray &data_in, model::ByteArray &data_out, uint32_t timeout = COMM_SESSION_DEFAULT_TIMEOUT, void (*notification)(float percent) = nullptr);

  /** Transmit a write only request to the remote pair */
  int request(uint8_t service, uint8_t cmde, const model::ByteArray &data_in, uint32_t timeout = COMM_SESSION_DEFAULT_TIMEOUT);

  template<class A>
  int register_cmde(uint8_t service,
                    uint8_t cmde,
                    A *target,
                    int (A:: *fun)(model::ByteArray &, model::ByteArray &));

  int register_cmde(uint8_t service, uint8_t cmde, CmdeFunctor *functor);

private:
  void client_thread();
  int request(uint8_t service, uint8_t cmde, const Packet &data_in, Packet &data_out, uint32_t timeout = COMM_SESSION_DEFAULT_TIMEOUT, void (*notification)(float percent) = nullptr);
  void on_event(const ComError &ce);
  void on_event(const Packet &p);
  bool session_error;

  hal::Mutex mutex_data_in;
  hal::Signal signal_data_in, signal_terminated;
  std::queue<Packet> data_in;
  uint32_t max_buffer_size;
  Transport *tp;
  hal::Mutex request_lock, response_lock;
  hal::Signal signal_answer;
  uint8_t service_waited;
  uint32_t response_max_length, response_length;
  Packet response;
  int do_terminate;
  /* For debug purpose */
  static Logable log;
  class CmdeStorage
  {
  public:
    uint8_t service;
    uint8_t cmde;
    CmdeFunctor *functor;
  };
  std::deque<CmdeStorage> cmde_handlers;
  hal::Mutex mutex;
  hal::Fifo<Packet> client_packets;
};


template<class A>
int Session::register_cmde(uint8_t service,
                           uint8_t code_cmde,
                           A *target,
                           int (A:: *fun)(model::ByteArray &, model::ByteArray &))
{
  /* TODO: check if not already registered */
  SpecificCmdeFunctor<A> *f = new SpecificCmdeFunctor<A>(target, fun);
  CmdeStorage storage;
  storage.service = service;
  storage.cmde    = code_cmde;
  storage.functor = f;
  cmde_handlers.push_back(storage);
  return 0;
}

}
}


#endif

