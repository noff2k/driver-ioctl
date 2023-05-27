#ifndef _DRIVER_H
#define _DRIVER_H

#include <iostream>
#include <Windows.h>

constexpr DWORD PROCESS_ID_CODE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

typedef struct _PROCESS_ID_REQUEST
{
  ULONG process_id;
  wchar_t process_name[260];
} PROCESS_ID_REQUEST, * PPROCESS_ID_REQUEST;

namespace driver
{
  inline HANDLE hDriver = nullptr;

  inline auto initialize() -> void
  {
    if ((hDriver = CreateFileW(L"\\\\.\\awesomedrivername222", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
      == INVALID_HANDLE_VALUE)
    {
      MessageBoxA(NULL, "The driver was not found.", "Error", MB_OK | MB_ICONERROR);
      ExitProcess(0);
    }
  }

  inline auto get_pid(const wchar_t* process_name, size_t process_name_size) -> ULONG
  {
    if (!hDriver)
      return 0;

    ULONG bytes = { 0 };
    PROCESS_ID_REQUEST process_id_request = { 0 };
    memcpy(process_id_request.process_name, process_name, process_name_size);

    if (DeviceIoControl(hDriver, PROCESS_ID_CODE, &process_id_request, sizeof(process_id_request), &process_id_request, sizeof(process_id_request), &bytes, 0))
      return process_id_request.process_id;
    else
      return 0;
  }
}

#endif