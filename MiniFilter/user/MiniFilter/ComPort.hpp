#ifndef COM_PORT_HPP
#define COM_PORT_HPP

#include <Windows.h>
#include <fltUser.h>

#include "utils.hpp"

typedef union __COM_MESSAGE {
  FILTER_MESSAGE_HEADER header;
  BYTE buffer[MAX_BUFFER_SIZE];
} COM_MESSAGE, *PCOM_MESSAGE;

class ComPort {
 public:
  ComPort();

  ~ComPort();

  HRESULT connect(std::wstring sPortName);

  HRESULT getMsg(std::wstring& wsMsg);

  HRESULT disconnect();

 private:
  std::wstring wsComPortName;

  HANDLE hComPort;
};

#endif