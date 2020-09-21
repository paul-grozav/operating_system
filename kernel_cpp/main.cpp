// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
class kernel
{
  void clear_screen();
  void print_string(const char *s);
  uint16_t p=0;
public:
  kernel();
  ~kernel();
  void run();
};
// -------------------------------------------------------------------------- //
void kernel::clear_screen()
{
  for(uint16_t i=0; i<80*25; i++)
  {
    *((uint16_t*)(0xB8000 + 2*i)) = ((uint16_t)(0x0F) << 8) | ' ';
  }
}
// -------------------------------------------------------------------------- //
void kernel::print_string(const char *s)
{
  for(uint16_t i=0; s[i] != 0; i++,p++)
  {
    *((uint16_t*)(0xB8000 + 2*p)) = ((uint16_t)(0x0F) << 8) | (s[i]);
  }
}
// -------------------------------------------------------------------------- //
kernel::kernel()
{
  clear_screen();
  const char* str = "KC | ";
  print_string(str);
}
// -------------------------------------------------------------------------- //
kernel::~kernel()
{
  const char* str = " | KD";
  print_string(str);
}
// -------------------------------------------------------------------------- //
void kernel::run()
{
  const char* str = "Hello world of C++ kernel!"
    " By Tancredi-Paul Grozav <paul@grozav.info>";
  print_string(str);
}
// -------------------------------------------------------------------------- //
extern "C" void kernel_main()
{
  kernel k;
  k.run();
}
// -------------------------------------------------------------------------- //
