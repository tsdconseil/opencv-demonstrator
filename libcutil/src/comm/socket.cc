#include "comm/socket.hpp"
#include "cutil.hpp"

#ifdef WIN
#include <windows.h>
#include <process.h>
#endif
#include <malloc.h>
#include <stdio.h>
#ifdef LINUX
#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <netinet/tcp.h>
static int WSAGetLastError(){return errno;}
#endif
#include <string.h>

namespace utils
{
namespace comm
{

#ifdef WIN
static WSAData wsa;
static bool winsock_init = false;
void winsock_startup()
{
  if(!winsock_init)
  {
    winsock_init = true;
    WSAStartup(MAKEWORD(1, 1), &wsa);
  }
}
#else
# define winsock_startup()
# endif

Socket::Socket(): rx_fifo(1024 * 1024)
{
  log.setup("comm/socket");
  connected = false;
}

Socket::~Socket()
{
  log.trace("Socket delete..");
  if(connected)
  {
    disconnect();
    hal::sleep(1);
  }
  log.trace("done.");
}

bool Socket::is_connected()
{
  return connected;
}


int Socket::disconnect()
{
  log.trace("closing socket..");
  connected = false;

  shutdown(sock, 2);

  int res = closesocket(sock);
  if(res)
    log.anomaly("closesocket error 0x%x.", res);

  char c = 0;
  rx_fifo.write(&c, 1);

  return 0;
}

int Socket::connect(std::string target_ip,
                    uint16_t target_port,
                    socket_type_t type)
{
  this->connected = false;

  winsock_startup();

  log.trace("connect(%s:%d)...", target_ip.c_str(), target_port);

  /* Socket creation */
  sockaddr_in local, remote;

  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  /* This is the port to connect from.  Setting 0 means use random port */
  local.sin_port = htons(0);

  remote.sin_family = AF_INET;
  remote.sin_port = htons(target_port);


  if(target_ip.compare("localhost") == 0)
  {
    //remote.sin_addr.s_addr/*.S_un.S_addr*/ = INADDR_ANY;
    //remote.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    target_ip = "127.0.0.1";
  }

# ifdef WIN
  if((remote.sin_addr.S_un.S_addr = inet_addr(target_ip.c_str())) == INADDR_NONE)
  {
    log.anomaly("Error setting IP.");
    return -1;
  }
# else
  struct hostent *server;
  server = gethostbyname(target_ip.c_str());
  if (!server)
  {
    log.anomaly("Impossible de résoudre \"%s\"", target_ip.c_str());
    return -1;
  }
  bzero(&remote, sizeof(remote));
  remote.sin_family = AF_INET;
  bcopy(server->h_addr, &remote.sin_addr.s_addr, server->h_length);
  remote.sin_port = htons(target_port);
# endif


  if(type == SOCKET_UDP)
  {
    int res = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(res == SOCKET_ERROR)
    {
      log.anomaly("Error creating socket.");
      return -1;
    }
    sock = res;
  }
  else if(type == SOCKET_TCP)
  {
    int res = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(res == SOCKET_ERROR)
    {
      log.anomaly("Error creating socket.");
      return -1;
    }
    sock = res;
  }
  else
  {
    log.anomaly("Unknown socket type: %d.", type);
    return -1;
  }

  if(::bind(sock, (sockaddr *)&local, sizeof(sockaddr_in)) == SOCKET_ERROR)
  {
    int err = WSAGetLastError();
    log.anomaly("Error binding socket, err = %x.", err);
    return -1;    
  }

  log.trace("Now connecting to server...");

  if(::connect(sock, (sockaddr *)&remote, sizeof(sockaddr_in)) == SOCKET_ERROR)
  {
    int err = WSAGetLastError();
    log.warning("Error connecting.");
    printf("Last error = 0x%x = %d.\n", err, err);
    return -1;    
  }
  this->connected = true;

# ifdef WIN
  u_long on = 1;
  ioctlsocket(sock, FIONBIO, &on);
# else
  const int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  //fcntl(sock, F_SETFL, O_NONBLOCK);
# endif
  log.trace("Connected ok.");
  hal::thread_start(this, &Socket::rx_thread, "socket/client");
  return 0;
}

void SocketServer::stop()
{
  if(listening)
  {
    log.trace("stop()...");
    listening = 0;
    shutdown(listen_socket, 2);
    thread_finished.wait(100);
  }
}

SocketServer::~SocketServer()
{
  stop();
}

void SocketServer::thread_handler()
{
  log.trace("Socket server thread running..");

  for(;;)
  {
    Socket *the_socket = new Socket();
    SOCKET accept_socket;
    sockaddr_in address;
    int remote_port;
    log.trace("Wait for client..");

#   ifdef LINUX
    socklen_t len = sizeof(sockaddr_in);
    accept_socket = ::accept(listen_socket, (struct sockaddr *) &address, &len);
#   else
    accept_socket = ::accept(listen_socket, nullptr, nullptr);
#   endif

    if(!listening)
    {
      thread_finished.raise();
      log.trace("accept thread terminated.");
      return;
    }

    if(((int) accept_socket) == -1)
    {
      thread_finished.raise();
      log.trace("accept thread terminated (ext).");
      return;
    }

    remote_port = ntohl(address.sin_port);

    log.trace("Connection accepted, sock = %x, remote port = %d.",
           accept_socket, listening, remote_port);


    the_socket->remote_port     = remote_port;
    the_socket->local_port      = local_port;
    the_socket->sock            = accept_socket;
    the_socket->connected = true;
    the_socket->socket_type     = Socket::SOCKET_TCP;
#   ifdef WIN
    u_long on = 1;
    ioctlsocket(the_socket->sock, FIONBIO, &on);
#   else
    const int flags = fcntl(the_socket->sock, F_GETFL, 0);
    fcntl(the_socket->sock, F_SETFL, flags | O_NONBLOCK);
#   endif
    hal::thread_start(the_socket, &Socket::rx_thread, "socket/server");
    SocketOpenedEvent soe;
    soe.socket = the_socket;
    dispatch(soe);
  }
}

SocketServer::SocketServer()
{
  log.setup("comm/socket-server");
  listening = 0;
}


int SocketServer::listen(uint16_t port, Socket::socket_type_t type)
{
  winsock_startup();

  local_port = port;

  log.trace("listen(port = %d).", port);


  service.sin_family = AF_INET;
  service.sin_addr.s_addr = INADDR_ANY;
  service.sin_port = htons(port);

  listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


  if(type == Socket::SOCKET_UDP)
  {
    listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  }
  else if(type == Socket::SOCKET_TCP)
  {
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  }
  else
  {
    log.anomaly("Unknown socket type: %d.", type);
    return -1;
  }

  if (listen_socket == INVALID_SOCKET)
  {
    log.anomaly("Error at socket(): %ld\n", WSAGetLastError());
    return -1;
  }

  {
    int flag = 1;
    if(setsockopt(listen_socket, SOL_SOCKET,
                  SO_REUSEADDR, (char *) &flag, sizeof(int)))
      log.anomaly("Failed to set SO_REUSEADDR socket option.");
  }

  if (::bind(listen_socket,
      (SOCKADDR*) &service,
      sizeof(service)) == SOCKET_ERROR)
  {
    log.anomaly("bind() failed.\n");
    uint32_t err = WSAGetLastError();
    log.anomaly("Last system error: %d = 0x%x\n", err, err);
#   ifdef LINUX
    if(err == 98)
    {
      log.warning("errno 98: Address already in use.");
    }
#   endif

    closesocket(listen_socket);
    return -1;
  }




  log.trace("Now listening for connection...");
  // Listen for incoming connection requests
  // on the created socket
  if (::listen(listen_socket, SOMAXCONN ) == SOCKET_ERROR)
  {
    int error = WSAGetLastError();
    log.anomaly("Error listening on socket: %d = 0x%x.\n", error, error);
    if(error == 10013)
    {
      log.anomaly("WSAEACCES: permission denied.");
    }

    return -1;
  }

  listening = 1;
  hal::thread_start(this, &SocketServer::thread_handler, "socket/server");
  return 0;
}

void Socket::rx_thread()
{
# define TMP_BUF_SIZE (32*1024)
  uint8_t *tmp_buf;
  //bool disable_timeout = false;

  log.trace("rx thread running.");
  
  
  if(!connected)
  {
    log.trace("rx thread canceled.");
    return;
  }

  tmp_buf = (uint8_t *) malloc(TMP_BUF_SIZE);

  if(tmp_buf == nullptr)
  {
    log.anomaly("Unable to allocate rx buffer.");
    return;
  }

  for(;;)
  {
    /* 5 ms */
    //timeval tv = { 0, 5 * 1000};

    //disable_timeout = true;

#   ifdef WIN
    FD_ZERO(&read_fs);
    FD_SET(sock, &read_fs);
    int res = select(1, &read_fs, 0, 0, nullptr/*disable_timeout ? nullptr : &tv*/);

    //disable_timeout = false;
    /* timeout */
    if(res == 0)
    {
      log.anomaly("Timeout");
      continue;
      //return -1;
    }
#   else

    FD_ZERO(&read_fs);
    FD_SET(sock, &read_fs);

    //verbose("select...");
    int res = select(sock + 1, &read_fs, 0, 0, nullptr);
    //verbose("select done.");

    /* timeout */
    if(res == 0)
    {
      log.anomaly("Timeout");
      continue;
    }

#   endif

    /*if(feof(sock))
    {
      warning("EOF detected.");
      connected = false;
      SocketClosedEvent sce;
      sce.socket = this;
      char tmp = 0;
      rx_fifo.write(&tmp, 1);
      dispatch(sce);
      free(tmp_buf);
      return;
      }*/


    {
      int result = recv(sock, (char *) tmp_buf, TMP_BUF_SIZE, 0);

      if(result > 0)
      {
        //if(result == TMP_BUF_SIZE)
        //  disable_timeout = true;

        //ByteArray ba(tmp_buf, result);
        //verbose("Rx: %s.", ba.to_string().c_str());

        rx_fifo.write(tmp_buf, result);
      }
      else if(result == 0)
      {
        log.warning("Connection closed.");
        connected = false;
        SocketClosedEvent sce;
        sce.socket = this;
        char tmp = 0;
        rx_fifo.write(&tmp, 1);
        this->rx_fifo.deblock();
        dispatch(sce);
        free(tmp_buf);
        return;
      }
      else
      {
#       ifdef LINUX
        if(errno == EAGAIN)
        {
          log.warning("recv: EAGAIN.");
          continue;
        }
#       endif

        log.warning("recv: returned %d.", result);

        int error = WSAGetLastError();

        if(!connected)
        {
          char c = 0xff;
          free(tmp_buf);
          
          log.trace("rx thread exit.");
          rx_fifo.write(&c, 1);
          this->rx_fifo.deblock();
          return;
        }

        log.warning("recv error: %d/%d (socket closed).", result, error);

#       ifdef WIN
        if(error == WSAEWOULDBLOCK)
        {
          log.anomaly("no data.");
          continue;
        }
#       endif

        if(result == -1)
        {
          log.warning("Connection closed.");
          connected = false;
          this->rx_fifo.deblock();
          SocketClosedEvent sce;
          sce.socket = this;
          dispatch(sce);
          free(tmp_buf);
          return;
        }
      }
    }
  }
}

int Socket::read(uint8_t *buffer, uint32_t length, int timeout)
{
  int res = rx_fifo.read(buffer, length, timeout);
  //ByteArray ba(buffer, length);
  //trace("Read: %s.", ba.to_string().c_str());
  return res;
}

int Socket::get_nb_rx_available()
{
  return rx_fifo.size();
}



int Socket::getc(int timeout)
{
  uint8_t c;
  int res;

  if(!connected)
  {
    if(timeout > 0)
      hal::sleep(timeout);

    hal::sleep(10);
    return -1;
  }

  // TODO: must be able to exit if the connection is closed.
  
  res = rx_fifo.read(&c, 1, timeout);

  if(!connected)
    return -1;

  //verbose("getc: %x, res = %d.", c, res);

  if(res == 1)
    return ((int) c) & 0xff;
  else
    return -1;
}



uint16_t Socket::get_local_port() const
{
  return local_port;
}

uint16_t Socket::get_remote_port() const
{
  return remote_port;
}

std::string Socket::get_remote_ip() const
{
  return remote_ip;
}


void Socket::putc(char c)
{

  write((uint8_t *) &c, 1);
}

void Socket::flush()
{
  int flag = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
  flag = 0;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
}

void Socket::write(const uint8_t *buffer, uint32_t len)
{
  uint32_t nb_tries = 0;
  const char *ptr = (const char *) buffer;
  bool inc_transmission = false;

  //ByteArray ba(buffer, len);
  //trace("Tx: %s.", ba.to_string().c_str());

  retry:
  if(this->connected)
  {
    int res = send(sock, ptr, len, 0);
    if(res < 0)
    {
      int err = WSAGetLastError();
      if(err == 10035)
      {
        if(nb_tries == 0)
          log.warning("TCP bandwidth overflow.");
        if(nb_tries > 50)
        {
          log.anomaly("Unable to write to TCP socket.");
          disconnect();
          return;
        }
        /*FD_ZERO(&write_fs);
        FD_SET(sock, &write_fs);
        int res = select(1, &write_fs, 0, 0, nullptr);*/
        hal::sleep(20);
        nb_tries++;
        goto retry;
      }
      log.anomaly("send error: %d.", err);
    }
    /*else if(res == 0)
    {
      anomaly("Incomplete send: %d / %d.", res, len);
    }*/
    else if(res < (int) len)
    {
      log.trace("Incomplete send: %d / %d.", res, len);
      hal::sleep(20);
      ptr    += res;
      len    -= res;
      inc_transmission = true;
      goto retry;
    }
    else if(inc_transmission)
    {
      log.trace("Finnaly transmitted all buffer.");
    }
  }
}



/*extern "C"
{
# ifdef WIN
  extern int bt_server_start(SOCKET *socket);
  extern int bt_client_connect(const char *target_mac, SOCKET *socket);
# if(defined(BT_SOCKET_DISABLE) && (BT_SOCKET_DISABLE == 1))
  int bt_server_start(SOCKET *socket){return -1;}
  int bt_client_connect(const char *target_mac, SOCKET *socket){return -1;}
# endif
# endif
};*/

typedef int (*bt_server_start_t)(SOCKET *socket,
                                 const char *service_name,
                                 const char *comment);

BluetoothClient::BluetoothClient()
{
  setup("comm/bluetooth-client");
}

BluetoothClient::~BluetoothClient()
{

}

int BluetoothClient::connect(const model::ByteArray &target_mac, Socket **socket)
{
  //int res;

  trace("Connect to %s...", target_mac.to_string().c_str());
  *socket = nullptr;
# if 0
# ifdef WIN
  SOCKET socket_windows;
  res = bt_client_connect(target_mac.to_string().c_str(), &socket_windows);
  if(res != 0)
  {
    anomaly("bt client connexion failed: %d.", res);
    return -1;
  }
  Socket *the_socket = new Socket();
  trace("Wait for client..");
  trace("Connection accepted.");
  warning("TODO: get remote port.");
  the_socket->remote_port     = 0x00;
  the_socket->local_port      = 0;
  the_socket->sock            = socket_windows;
  the_socket->connected       = true;
  the_socket->socket_type     = Socket::SOCKET_BT;
  u_long on = 1;
  ioctlsocket(the_socket->sock, FIONBIO, &on);
  OSThread::thread_start(the_socket, &Socket::rx_thread);
  return 0;
# else
  warning("connect: not implemented.");
  return -1;
# endif
# endif
  return -1;
}

BluetoothServer::BluetoothServer(const std::string &service_name,
                                 const std::string &comment)
{
  setup("comm/bluetooth-server");
  this->service_name = service_name;
  this->comment      = comment;
}

int BluetoothServer::listen()
{
  //OSThread::thread_start(this, &BluetoothServer::thread_handler);
  int res;

  trace("Listen...");

#   ifdef WIN
  HINSTANCE hdll;
  hdll = LoadLibrary("blue.dll");

  if(hdll == nullptr)
  {
    anomaly("Error while loading dll.\n");
    return -254;
  }


  bt_server_start_t bt_server_start = (bt_server_start_t) GetProcAddress(hdll, (LPCSTR) 2);
      //"_bt_server_start");

  if(bt_server_start == nullptr)
  {
    anomaly("Error while loading DLL function.\n");
    return -253;
  }

  trace("calling DLL..");
  res = bt_server_start(&listen_socket,
                        service_name.c_str(),
                        comment.c_str());
#   else
  res = -255;
#   endif


  if(res == 0)
  {

    trace("Bluetooth server successfully started.");
    hal::thread_start(this, &BluetoothServer::thread_handler, "btsocket/server");
    /*Socket *the_socket = new Socket();
    the_socket->remote_port     = 0x00;
    the_socket->local_port      = 0;
    the_socket->sock            = socket_windows;
    the_socket->connected       = true;
    the_socket->socket_type     = Socket::SOCKET_BT;
    u_long on = 1;
    ioctlsocket(the_socket->sock, FIONBIO, &on);
    OSThread::thread_start(the_socket, &Socket::rx_thread);
    SocketOpenedEvent soe;
    soe.socket = the_socket;
    dispatch(soe);*/
  }
  else if(res == -3)
  {
    anomaly("Bluetooth driver not detected.");
  }
  else
  {
    anomaly("Bluetooh server error: %d.", res);
  }

  return res;
}

void BluetoothServer::stop()
{

}

void BluetoothServer::thread_handler()
{
  trace("bluetooth server is running.");
  for(;;)
  {
    Socket *the_socket = new Socket();
    SOCKET accept_socket;
    trace("Wait for client..");
    accept_socket = ::accept(listen_socket, nullptr, nullptr);
    trace("Connection accepted.");
    the_socket->remote_port     = 0x00;
    //the_socket->local_port      = local_port;
    the_socket->sock            = accept_socket;
    the_socket->connected = true;
    the_socket->socket_type     = Socket::SOCKET_BT;
#   ifdef WIN
    u_long on = 1;
    ioctlsocket(the_socket->sock, FIONBIO, &on);
#   else
    fcntl(the_socket->sock, F_SETFL, O_NONBLOCK);
#   endif
    hal::thread_start(the_socket, &Socket::rx_thread, "btsocket/server");
    SocketOpenedEvent soe;
    soe.socket = the_socket;
    dispatch(soe);
  }
}


int send_udp_packet(const std::string &host,
                    uint16_t port,
                    utils::model::ByteArray &data_)
{
  struct hostent *hp;     /* host information */
  struct sockaddr_in servaddr;    /* server address */

  uint32_t len = data_.size();
  uint8_t *data = (uint8_t *) malloc(len);
  if(data == nullptr)
  {
    log_anomaly(0, "%s: malloc failed (%d).", __func__, len);
    return -1;
  }

  data_.pop_data(data, len);

  winsock_startup();

  /* fill in the server's address and data */
  memset((char*)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  /* look up the address of the server given its name */
  hp = gethostbyname(host.c_str());
  if(!hp)
  {
    free(data);
    log_anomaly(0, "could not obtain address of %s.", host.c_str());
    return -1;
  }

  /* put the host's address into the server address structure */
  memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

  SOCKET fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(fd == INVALID_SOCKET)
  {
    free(data);
    log_anomaly(0, "socket creation failed with error: %ld", WSAGetLastError());
    return -1;
  }

  /* send a message to the server */
  if(sendto(fd,
            (const char *) data,
            len,
            0,
            (struct sockaddr *)&servaddr,
            sizeof(servaddr)) == SOCKET_ERROR)
  {
    free(data);
    perror("sendto failed");
    log_anomaly(0, "Failed to send udp packet.");
    return -1;
  }

  closesocket(fd);

  free(data);
  return 0;
}



UDPListener::UDPListener()
{
  log.setup("udp/listener");
  listening = false;
}




int UDPListener::listen(uint16_t port, uint32_t mps)
{
  winsock_startup();

  if(listening)
  {
    log.anomaly("%s: already listening.", __func__);
    return -1;
  }

  log.trace("listen(port = %d)..", port);

  this->mps = mps;
  struct sockaddr_in myaddr;      /* our address */

  /* create a UDP socket */
  //if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(fd == SOCKET_ERROR)
  {
    perror("");
    log.anomaly("cannot create socket: %ld.", WSAGetLastError());
    return -1;
  }

  /* bind the socket to any valid IP address and a specific port */

  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(port);

  if(bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == SOCKET_ERROR)
  {
    perror("");
    log.anomaly("bind failed: %ld.", WSAGetLastError());
    closesocket(fd);
    return -1;
  }

  buf = (uint8_t *) malloc(mps);
  if(buf == nullptr)
  {
    log.anomaly("malloc failed.");
    closesocket(fd);
    return -1;
  }

  listening = true;
  utils::hal::thread_start(this, &UDPListener::thread_entry);
  return 0;
}

UDPListener::~UDPListener()
{
  if(listening)
  {
    // TODO
    closesocket(fd);
  }
}

void UDPListener::thread_entry()
{
  /* # bytes received */
  int len;
  /* remote address */
  struct sockaddr_in remaddr;
  /* length of addresses */
  #ifdef LINUX
socklen_t
# else
int
# endif
   addrlen = sizeof(remaddr);

  for(;;)
  {
    len = recvfrom(fd,
                   (char *) buf,
                   mps, 0,
                   (struct sockaddr *)&remaddr, &addrlen);

    if(len == (int) INVALID_SOCKET)
    {
      log.anomaly("WSA error: %ld", WSAGetLastError());
    }

    log.verbose("received a packet of %d bytes.", len);
    if(len > 0)
    {
      UDPPacket packet;
      packet.data.put(buf, len);
      packet.ip_address = inet_ntoa(remaddr.sin_addr);
      dispatch(packet);
    }
  }
  /* never exits */
}




}
}


