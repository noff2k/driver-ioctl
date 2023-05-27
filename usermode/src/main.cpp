#include "driver.h"

auto main(void) -> int
{
  driver::initialize();
  const wchar_t process[] = L"cs2.exe";
  std::printf("%u", driver::get_pid(process, sizeof(process)));
  return 0;
}