#include "../../include/journal.hpp"
#include "comm/serial.hpp"
#ifdef WIN
#include <windows.h>
#endif

namespace utils
{
namespace comm
{

#ifdef WIN
int reg_open_subkey_at(HKEY hKey, uint32_t index, REGSAM samDesired,
                       PHKEY phkResult, std::string *subkey_name = nullptr)
{
  uint32_t size = 256;
  char *buffer;
  int errcode;
  DWORD cbSubkeyName = 128 * sizeof(TCHAR);
  FILETIME filetime;

  /* loop asking for the subkey name til we allocated enough memory */

  for (;;)
  {
    buffer = (char *) malloc(size);
    if(buffer == nullptr)
    {
      return -1;
    }
    errcode = RegEnumKeyEx(hKey, index, buffer, &cbSubkeyName,
                           0, nullptr, nullptr, &filetime);

    if(errcode == ERROR_MORE_DATA)
    {
      free(buffer);
      size *= 2;
      continue;
    }

    if(errcode != 0)
    {
      if(errcode != ERROR_NO_MORE_ITEMS)
        erreur("RegEnumKeyEx error %d, index = %d.\n", errcode, index);
      free(buffer);
      return errcode;
    }
    break;
  }
  if(subkey_name != nullptr)
    *subkey_name = std::string(buffer);
  errcode = RegOpenKeyEx(hKey, buffer, 0, samDesired, phkResult);
  if(errcode != 0)
    erreur("RegOpenKeyEx error %d.\n", errcode);
  free(buffer);
  return errcode;
}

int reg_query_string_value(HKEY hKey, std::string name, std::string &value)
{
  uint32_t size = 256;
  int errcode;
  char *buffer;

  for(;;)
  {
    buffer = (char *) malloc(size);
    if(buffer == nullptr)
    {
      return -1;
    }
    errcode = RegQueryValueEx(hKey, name.c_str(), nullptr, nullptr,
                              (uint8_t *) buffer, (DWORD*)&size);

    if(errcode == 0)
    {
      value = std::string(buffer);
      free(buffer);
      return 0;
    }
    else if(errcode == ERROR_MORE_DATA)
    {
      size *= 2;
      free(buffer);
      continue;
    }
    free(buffer);
    return -1;
  }
}
#endif

int Serial::enumerate(std::vector<SerialInfo> &infos)
{
# ifdef LINUX
  return 0;
# else
  std::vector<SerialInfo> tmp;
  OSVERSIONINFO os;
  uint32_t i, j, k, errcode;
  HKEY key_enum, key1, key2, key3;
  std::string port_name, friendly_name, technology;

  infos("Serial port enumeration...");

  memset(&os, 0, sizeof(os));
  os.dwOSVersionInfoSize = sizeof(os);

  GetVersionEx(&os);

  if ((os.dwPlatformId != VER_PLATFORM_WIN32_NT) || (os.dwMajorVersion <= 4))
  {
    erreur("Unsupported os.");
    return -1;
  }

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CURRENTCONTROLSET\\ENUM", 0,
      KEY_ENUMERATE_SUB_KEYS, &key_enum) != 0)
  {
    erreur("enumerate: RegOpenKeyEx error.");
    return -1;
  }

  for(i = 0;; i++)
  {
    errcode = reg_open_subkey_at(key_enum, i, KEY_ENUMERATE_SUB_KEYS, &key1, &technology);

    if(errcode == ERROR_NO_MORE_ITEMS)
      break;
    else if(errcode != 0)
    {
      erreur("enumerate: reg_open_subkey_at(1) error.");
      return -1;
    }

    for(j = 0; ; j++)
    {
      errcode = reg_open_subkey_at(key1, j, KEY_ENUMERATE_SUB_KEYS, &key2, nullptr);
      if(errcode == ERROR_NO_MORE_ITEMS)
        break;
      else if(errcode != 0)
      {
        erreur("enumerate: reg_open_subkey_at(2) error.");
        RegCloseKey(key1);
        RegCloseKey(key_enum);
        return -1;
      }

      for(k = 0;; k++)
      {
        errcode = reg_open_subkey_at(key2, k, KEY_READ, &key3, nullptr);
        if (errcode == ERROR_NO_MORE_ITEMS)
          break;
        else if(errcode != 0)
        {
          erreur("enumerate: reg_open_subkey_at(3) error.");
          RegCloseKey(key1);
          RegCloseKey(key2);
          RegCloseKey(key_enum);
          return -1;
        }

        char buf[50];
        uint32_t bsize = 50;

        if ((RegQueryValueEx(key3, "CLASS", nullptr, nullptr, (uint8_t*) buf, (DWORD*)&bsize) == 0)
            && (strcmp(buf, "PORTS")))
        {
          // Ok
        }
        else if ((RegQueryValueEx(key3, "CLASSGUID", nullptr, nullptr, (uint8_t*) buf,
            (DWORD*)&bsize) == 0) && (strcmp(buf,
            "{4D36E978-E325-11CE-BFC1-08002BE10318}")))
        {

        }
        else
        {
          continue;
        }

        if(reg_query_string_value(key3, "PORTNAME", port_name) != 0)
        {
          HKEY key_dev_param;
          if (RegOpenKeyEx(key3,
                           "DEVICE PARAMETERS",
                           0,
                           KEY_READ,
                           &key_dev_param) != 0)
          {
            continue;
          }
          if(reg_query_string_value(key_dev_param, "PORTNAME", port_name) != 0)
          {
            RegCloseKey(key_dev_param);
            continue;
          }
          RegCloseKey(key_dev_param);
        }

        /* check if it is a serial port (instead of, say, a parallel port) */
        if ((port_name[0] != 'C') || (port_name[1] != 'O') || (port_name[2]
            != 'M'))
          continue;


        reg_query_string_value(key3, "FRIENDLYNAME", friendly_name);

        SerialInfo serial_info;
        serial_info.name = port_name;
        serial_info.complete_name = (friendly_name.size() == 0) ? serial_info.name : std::string(friendly_name);
        serial_info.techno = technology;
        tmp.push_back(serial_info);

        RegCloseKey(key3);
      } // for k
      RegCloseKey(key2);
    } // for j
    RegCloseKey(key1);
  } // for i
  RegCloseKey(key_enum);

  // Vérifie que les ports COM sont bien actifs
  for(auto &si: tmp)
  {
    char port[50];

    sprintf(port, "%s", si.name.c_str());
    if(strlen(port) > 4)
    {
      sprintf(port, "%s%s", "\\\\.\\", si.name.c_str());
    }

    HANDLE h = ::CreateFile(
                      port,
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
    if(h == INVALID_HANDLE_VALUE)
    {
      avertissement("Enumeration : port COM [%s] non actif.", si.name.c_str());
    }
    else
    {
      infos("Enumeration : port COM [%s] actif.", si.name.c_str());
      infos.push_back(si);
      ::CloseHandle(h);
    }
  }



  return 0;
# endif
}

}
}

