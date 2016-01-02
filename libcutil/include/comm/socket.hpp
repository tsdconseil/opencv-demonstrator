#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "comm/iostreams.hpp"

#include "trace.hpp"
#include "slots.hpp"
#include "cutil.hpp"

#include <string>
#include <stdint.h>
#ifdef WIN
#include "winsock2.h"
#elif defined(LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#endif

namespace utils
{
namespace comm
{

using namespace utils;


class Socket;
class BluetoothServer;
class BluetoothClient;

struct SocketClosedEvent
{
  Socket *socket;
};

struct SocketOpenedEvent
{
  Socket *socket;
};


extern int send_udp_packet(const std::string &host,
                           uint16_t port,
                           utils::model::ByteArray &data);


struct UDPPacket
{
  utils::model::ByteArray data;
  std::string ip_address;
};

class UDPListener: public CProvider<UDPPacket>
{
public:
  UDPListener();
  int listen(uint16_t port, uint32_t mps = 4096);
  ~UDPListener();
private:
  bool listening;
  /* our socket */
  int fd;
  uint8_t *buf;
  uint32_t mps;

  Logable log;
  void thread_entry();
};

class Socket: public  IOStream,
              public  CProvider<SocketClosedEvent>
{
public:
  typedef enum socket_type_enum
  {
    SOCKET_UDP = 0,
    SOCKET_TCP = 1,
    SOCKET_BT  = 2
  } socket_type_t;

  Socket();
  virtual ~Socket();
  bool is_connected();
  int connect(std::string target_ip,
              uint16_t target_port,
              socket_type_t type = SOCKET_TCP);
  int disconnect();
  
  void write(const uint8_t *buffer, uint32_t len);
  int read(uint8_t *buffer, uint32_t length, int timeout);
  int getc(int timeout = 0);
  void putc(char c);
  void flush();

  uint16_t get_local_port() const;
  uint16_t get_remote_port() const;
  std::string get_remote_ip() const;

  friend class SocketServer;
  friend class BluetoothServer;
  friend class BluetoothClient;
private:
  Logable log;
  void rx_thread();
  hal::RawFifo rx_fifo;
  socket_type_t socket_type;
  uint16_t local_port, remote_port;
  std::string remote_ip;
  bool connected;
  SOCKET sock;
  fd_set read_fs, write_fs;
# ifdef WIN
# else
# endif
};

class SocketServer: public  CProvider<SocketOpenedEvent>
{
public:
  SocketServer();
  ~SocketServer();

  /** Open a server port and listen for clients */
  int listen(uint16_t port, Socket::socket_type_t type = Socket::SOCKET_TCP);

  /** Stop listening for new clients */
  void stop();

private:
  void thread_handler();
  Logable log;
  SOCKET listen_socket;
  sockaddr_in service;
# ifdef WIN
  WSAData wsa;
# endif
  uint32_t local_port;
  Socket::socket_type_t socket_type;
  int listening;
  hal::Signal thread_finished;
};


class BluetoothServer: private Logable,
                       public  CProvider<SocketOpenedEvent>
{
public:
  BluetoothServer(const std::string &service_name,
                  const std::string &comment);

  /** Open a server port and listen for clients
   *  @returns:
   *     0  ok
   *  -255  Not supported on this target
   *  -254  blue.dll not found
   *  -253  Routine not found in blue.dll (invalid DLL)
   *    -3  No bluetooth hardware on the host target (bind error)
   *    -6  Register bt service failed
   **/
  int listen();
  /** Stop listening for new clients */
  void stop();

private:
  void thread_handler();
  SOCKET listen_socket;
  std::string service_name;
  std::string comment;
};

class BluetoothClient: private Logable
{
public:
  BluetoothClient();
  ~BluetoothClient();

  int connect(const model::ByteArray &target_mac, Socket **socket);
private:
};

#if 0
class BluetoothSocket: private Logable,
                       public  IOStream
{
public:
  BluetoothSocket(){}
  /** Open a server port and listen for clients */
  int listen();
  /** Stop listening for new clients */
  void stop();

  bool is_connected();
  int connect(std::string target_mac);
  int disconnect();

  void write(const uint8_t *buffer, uint32_t len);
  int read(uint8_t *buffer, uint32_t length, int timeout);
  int getc(int timeout = 0);
  void putc(char c);

private:
  void thread_handler();
  SOCKET listen_socket;
  sockaddr_in service;
  WSAData wsa;
  uint32_t local_port;
  Socket::socket_type_t socket_type;
};
#endif


}
}

#endif
