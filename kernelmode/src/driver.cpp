#include "defines.h"

UNICODE_STRING deviceName, symLink;
PDEVICE_OBJECT pDeviceObject;

#define to_lower(c_char) ((c_char >= 'A' && c_char <= 'Z') ? (c_char + 32) : c_char)

template <typename str_type, typename str_type_2>
bool crt_strcmp(str_type str, str_type_2 in_str, bool two)
{
  if (!str || !in_str)
    return false;
  wchar_t c1, c2;
  do
  {
    c1 = *str++; c2 = *in_str++;
    c1 = to_lower(c1); c2 = to_lower(c2);
    if (!c1 && (two ? !c2 : 1))
      return true;
  } while (c1 == c2);
  return false;
}

HANDLE GetProcessByName(const wchar_t* process_name)
{
  CHAR image_name[15];
  PEPROCESS sys_process = PsInitialSystemProcess;
  PEPROCESS cur_entry = sys_process;
  do
  {
    RtlCopyMemory((PVOID)(&image_name), (PVOID)((uintptr_t)cur_entry + 0x5A8), sizeof(image_name));
    if (crt_strcmp(image_name, process_name, true))
    {
      unsigned long active_threads;
      RtlCopyMemory((PVOID)&active_threads, (PVOID)((uintptr_t)cur_entry + 0x5F0), sizeof(active_threads));
      if (active_threads)
        return PsGetProcessId(cur_entry);
    }
    PLIST_ENTRY list = (PLIST_ENTRY)((uintptr_t)(cur_entry) + 0x448);
    cur_entry = (PEPROCESS)((uintptr_t)list->Flink - 0x448);
  } while (cur_entry != sys_process);
  return 0;
}

NTSTATUS HandleIO(PDEVICE_OBJECT device_obj, PIRP irp)
{
  UNREFERENCED_PARAMETER(device_obj);

  NTSTATUS status = STATUS_INVALID_PARAMETER;
  ULONG bytes_io = 0;
  PIO_STACK_LOCATION pio = IoGetCurrentIrpStackLocation(irp);
  ULONG ioctl = pio->Parameters.DeviceIoControl.IoControlCode;

  if (ioctl == PROCESS_ID_CODE)
  {
    PPROCESS_ID_REQUEST process_id_request = (PPROCESS_ID_REQUEST)irp->AssociatedIrp.SystemBuffer;

    process_id_request->process_id = (ULONG)GetProcessByName(process_id_request->process_name);
    if (process_id_request->process_id)
      status = STATUS_SUCCESS;
    bytes_io = sizeof(PROCESS_ID_REQUEST);
  }
  else
  {
    status = STATUS_INVALID_PARAMETER;
    bytes_io = 0;
  }
  irp->IoStatus.Status = status;
  irp->IoStatus.Information = bytes_io;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return status;
}

NTSTATUS UnsupportedIO(PDEVICE_OBJECT device_obj, PIRP irp)
{
  UNREFERENCED_PARAMETER(device_obj);
  irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return irp->IoStatus.Status;
}

NTSTATUS CreateIO(PDEVICE_OBJECT device_obj, PIRP irp)
{
  UNREFERENCED_PARAMETER(device_obj);
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return irp->IoStatus.Status;
}

NTSTATUS CloseIO(PDEVICE_OBJECT device_obj, PIRP irp)
{
  UNREFERENCED_PARAMETER(device_obj);
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return irp->IoStatus.Status;
}

void UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
  IoDeleteSymbolicLink(&symLink);
  IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS Run(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
  UNREFERENCED_PARAMETER(pRegistryPath);

  RtlInitUnicodeString(&deviceName, L"\\Device\\awesomedrivername222");
  auto status = IoCreateDevice(pDriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
  if (status != STATUS_SUCCESS)
    return status;

  RtlInitUnicodeString(&symLink, L"\\DosDevices\\awesomedrivername222");
  status = IoCreateSymbolicLink(&symLink, &deviceName);
  if (status != STATUS_SUCCESS)
    return status;

  SetFlag(pDriverObject->Flags, DO_BUFFERED_IO);

  for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    pDriverObject->MajorFunction[i] = UnsupportedIO;

  pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateIO;
  pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseIO;
  pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleIO;
  pDriverObject->DriverUnload = UnloadDriver;

  ClearFlag(pDeviceObject->Flags, DO_DEVICE_INITIALIZING);
  return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path)
{
  UNREFERENCED_PARAMETER(driver_obj);
  UNREFERENCED_PARAMETER(registery_path);

  UNICODE_STRING name;
  RtlInitUnicodeString(&name, L"\\Driver\\awesomedrivername222");
  IoCreateDriver(&name, &Run);
  return STATUS_SUCCESS;
}