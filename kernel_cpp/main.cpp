// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
namespace os{
// -------------------------------------------------------------------------- //
class kernel
{
  void clear_screen();
  void print_string(const char *s);
  void print_character(const char c);
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
    print_character(' ');
  }
  p = 0; // reset position
}
// -------------------------------------------------------------------------- //
void kernel::print_string(const char *s)
{
  for(uint16_t i=0; s[i] != 0; i++)
  {
//    const uint16_t value = ((uint16_t)(0x0F) << 8) | (s[i]);
    print_character(s[i]);
  }
}
// -------------------------------------------------------------------------- //
void kernel::print_character(const char c)
{
  // The text screen video memory for colour monitors resides at 0xB8000,
  // and for monochrome monitors it is at address 0xB0000
  const uint32_t colour_monitor_video_memory = 0xB8000;

  // This is calculated in a 64bits integer, even though the address should
  // fit in a 32bits integer.
  const uint64_t memory_address = colour_monitor_video_memory + 2*p;
  uint16_t * const memory_pointer = (uint16_t*)(memory_address);
  const uint16_t value = ((uint16_t)(0x0F) << 8) | c;
  *memory_pointer = value;
  p = p + 1;
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
  const char* str = "Hello world of C++ Kernel"
    " By Tancredi-Paul Grozav <paul@grozav.info>";
  print_string(str);
}
// -------------------------------------------------------------------------- //
}; // end of namespace ::os
// -------------------------------------------------------------------------- //
extern "C" void kernel_main()
{
  ::os::kernel k;
  k.run();
}
// -------------------------------------------------------------------------- //
