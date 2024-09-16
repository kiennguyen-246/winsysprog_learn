#ifndef COM_PORT_HPP
#define COM_PORT_HPP

#include <Windows.h>
#include <fltUser.h>

#include "utils.hpp"
#include "public/public.h"

typedef struct _COM_MESSAGE {
  FILTER_MESSAGE_HEADER header;
  MFLT_EVENT_RECORD eventRecord;
} COM_MESSAGE, *PCOM_MESSAGE;

class ComPort {
 public:
  ComPort();

  ~ComPort();

  HRESULT connect(std::wstring sPortName);

  HRESULT getRecord(PMFLT_EVENT_RECORD pEventRecord);

  HRESULT disconnect();

 private:
  std::wstring wsComPortName;

  HANDLE hComPort;
};

#endif