/** @file btsocket_vsudio.cc */
#include <stdafx.h>
#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <strsafe.h>
#include <intsafe.h>
#include <stdio.h>
#include <Objbase.h>
#include "blue.h"
#include <string.h>
#include <cguid.h>

//static FILE *flog;

#define trace(...)

int _bt_server_start(SOCKET *socket_,
                     const char *service_name,
                     const char *comment)
{
  //flog = fopen("bluelog.txt", "wt");
  trace("Bluetooth socket server test, peripheral = '%s'.\n", "peripheral");

  WSADATA m_data;
  /* Load the winsock2 library */
  if (WSAStartup(MAKEWORD(2,2), &m_data) != 0)
  {
    trace("wsastartup error.\n");
    return -1;
  }

  SOCKET s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

  if (s == INVALID_SOCKET)
  {
    trace("Could not create socket: error %d\n", WSAGetLastError());
    return -1;
  }


     WSAPROTOCOL_INFO protocolInfo;
   int protocolInfoSize = sizeof(protocolInfo);

    if (0 != getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO,
		(char*)&protocolInfo, &protocolInfoSize))
    {
    trace("getsockopt: error %d\n", WSAGetLastError());
    return -1;
    }

     SOCKADDR_BTH address;
     int sa_len = sizeof(SOCKADDR_BTH);
   address.addressFamily = AF_BTH;
   address.btAddr = 0;
   address.serviceClassId = GUID_nullptr;
   address.port = BT_PORT_ANY;
   sockaddr *pAddr = (sockaddr*)&address;

  if (bind(s, pAddr, sizeof(SOCKADDR_BTH)))
  {
    trace("bind error %d\n", WSAGetLastError());
    closesocket(s);
    return -3;
  }

  if(listen(s, 10))//5))
  {
    trace("listen error %d\n", WSAGetLastError());
    closesocket(s);
    return -1;
  }

  // check which port were listening on
  if(getsockname(s, (SOCKADDR*)&address, &sa_len))
  {
    trace("getsockname error %d\n", WSAGetLastError());
    closesocket(s);
    return -1;
  }

  trace("listening on RFCOMM port: %d\n" , address.port) ;

  trace("Registering SDP service (METHOD 0)...\n");
  WSAQUERYSET service;

  memset(&service, 0, sizeof(service));

  service.dwSize = sizeof(service);

  service.lpszServiceInstanceName = strdup(service_name);
    //_T("My Service");
  service.lpszComment = strdup(comment);
    //_T("My comment");

  // UUID for SPP is 00001101-0000-1000-8000-00805F9B34FB
  GUID serviceID = /*(GUID)*/SerialPortServiceClass_UUID;

  service.lpServiceClassId = &serviceID;

  service.dwNumberOfCsAddrs = 1;
  service.dwNameSpace = NS_BTH;

  CSADDR_INFO csAddr;
  memset(&csAddr, 0, sizeof(csAddr));
  csAddr.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
  csAddr.LocalAddr.lpSockaddr = pAddr;//(LPSOCKADDR) &sa;
  csAddr.iSocketType = SOCK_STREAM;
  csAddr.iProtocol = BTHPROTO_RFCOMM;
  service.lpcsaBuffer = &csAddr;

  if (0 != WSASetService(&service, RNRSERVICE_REGISTER, 0))
  {
    trace("set service error:%d\n", WSAGetLastError());
    closesocket(s);
    return -6;
  }


  trace("Waiting for client connection...\n");
  //fflush(flog);
  // Sur la carte d'évaluation, faire :
  // CALL 00:0A:3A:7F:24:69 1101 RFCOMM

# if 0
  SOCKADDR_BTH sa2;
  int size = sizeof(sa2);
  SOCKET s2 = accept(s, (SOCKADDR *)&sa2, &size);

  if(s2 == INVALID_SOCKET)
  {
    fprintf(flog, "accept error %d\n", WSAGetLastError());
    closesocket(s);
    return -7;
  }
  fprintf(flog, "Connected!\n");
  fflush(flog);
# endif



  *socket_ = s;
  //fclose(flog);
  return 0;
}


#if 0
int _bt_server_start(SOCKET *socket_,
                     const char *service_name,
                     const char *comment)
{
  flog = fopen("bluelog.txt", "wt");
  fprintf(flog, "Bluetooth socket server test, peripheral = '%s'.\n", "peripheral");
  fflush(flog);

  WSADATA m_data;
  /* Load the winsock2 library */
  if (WSAStartup(MAKEWORD(2,2), &m_data) != 0)
  {
    fprintf(flog, "wsastartup error.\n");
    fclose(flog);
    return -1;
  }

  SOCKET s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

  if (s == INVALID_SOCKET)
  {
    fprintf(flog, "Could not create socket: error %d\n", WSAGetLastError());
    return -1;
  }

  SOCKADDR_BTH sa;
  int sa_len = sizeof(sa);
  memset (&sa, 0, sizeof(sa));
  sa.addressFamily = AF_BTH;
  sa.port = BT_PORT_ANY;

  if (bind(s, (SOCKADDR *)&sa, sizeof(sa)))
  {
    fprintf(flog, "bind error %d\n", WSAGetLastError());
    closesocket(s);
    return -3;
  }

  if(listen(s, 5))
  {
    fprintf(flog, "listen error %d\n", WSAGetLastError());
    closesocket(s);
    return -1;
  }

  // check which port were listening on
  if(getsockname(s, (SOCKADDR*)&sa, &sa_len))
  {
    fprintf(flog, "getsockname error %d\n", WSAGetLastError());
    closesocket(s);
    return -1;
  }

  fprintf(flog, "listening on RFCOMM port: %d\n" , sa.port) ;
  fflush(flog);

  fprintf(flog, "Registering SDP service (METHOD 0)...\n");
  fflush(flog);
  WSAQUERYSET service;

  memset(&service, 0, sizeof(service));

  service.dwSize = sizeof(service);

  service.lpszServiceInstanceName = strdup(service_name);
    //_T("My Service");
  service.lpszComment = strdup(comment);
    //_T("My comment");

  // UUID for SPP is 00001101-0000-1000-8000-00805F9B34FB
  GUID serviceID = (GUID)SerialPortServiceClass_UUID;

  service.lpServiceClassId = &serviceID;

  service.dwNumberOfCsAddrs = 1;
  service.dwNameSpace = NS_BTH;

  CSADDR_INFO csAddr;
  memset(&csAddr, 0, sizeof(csAddr));
  csAddr.LocalAddr.iSockaddrLength = sizeof(SOCKADDR);
  csAddr.LocalAddr.lpSockaddr = (LPSOCKADDR) &sa;
  csAddr.iSocketType = SOCK_STREAM;
  csAddr.iProtocol = BTHPROTO_RFCOMM;

  service.lpcsaBuffer = &csAddr;

  if (0 != WSASetService(&service, RNRSERVICE_REGISTER, 0))
  {
    printf("set service error:%d\n", WSAGetLastError());
    closesocket(s);
    return -6;
  }


  fprintf(flog, "Waiting for client connection...\n");
  fflush(flog);
  // Sur la carte d'évaluation, faire :
  // CALL 00:0A:3A:7F:24:69 1101 RFCOMM

# if 1
  SOCKADDR_BTH sa2;
  int size = sizeof(sa2);
  SOCKET s2 = accept(s, (SOCKADDR *)&sa2, &size);

  if(s2 == INVALID_SOCKET)
  {
    fprintf(flog, "accept error %d\n", WSAGetLastError());
    closesocket(s);
    return -7;
  }
# endif

  fprintf(flog, "Connected!\n");
  fflush(flog);

  *socket_ = s;
  fclose(flog);
  return 0;
}
#endif




int _bt_client_connect(const char *target_mac, SOCKET *socket_)
{
  printf("Bluetooth socket client test, peripheral = '%s'.\n", "peripheral");

  WSADATA m_data;
  /* Load the winsock2 library */
  if (WSAStartup(MAKEWORD(2,2), &m_data) != 0)
  {
    printf("wsastartup error.\n");
    return -1;
  }

    SOCKET s = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

    if (s == INVALID_SOCKET)
    {
      printf("Could not create socket: error %d\n", WSAGetLastError());
      return -2;
    }

    SOCKADDR_BTH sa;
    memset (&sa, 0, sizeof(sa));
    sa.addressFamily = AF_BTH;

    /** Adresse de la carte d'évaluation bluegiga (inversée) */
    unsigned char peripheral_address[6] = {0x75,0x2b,0x81,0x80,0x07,0x00};

    memcpy(&(sa.btAddr), peripheral_address, 6);

    unsigned char service_guid[16] = {0x0,0x1,0,0,0,0,0,0x10,0x80,0,0,0x80,0x5f,0x9b,0x34,0xfb};

    memcpy(&(sa.serviceClassId), service_guid, 16);

    if(connect(s, (SOCKADDR *) &sa, sizeof(sa)) != 0)
    {
      printf("Error during connection: %d\n", WSAGetLastError());
      return -3;
    }

    printf("Successfully connected.\n");

    *socket_ = s;
    return 0;
}
