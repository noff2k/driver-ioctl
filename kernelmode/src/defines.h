#ifndef _DEFINES_H
#define _DEFINES_H

#include <ntifs.h>

constexpr ULONG PROCESS_ID_CODE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

typedef struct _PROCESS_ID_REQUEST
{
  ULONG process_id;
  wchar_t process_name[260];
} PROCESS_ID_REQUEST, * PPROCESS_ID_REQUEST;

extern "C"
NTKERNELAPI
NTSTATUS
IoCreateDriver(
  PUNICODE_STRING DriverName,
  PDRIVER_INITIALIZE InitializationFunction
);

#endif
