// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include <stdint.h> // uintX
#include "module_base.h"
// -------------------------------------------------------------------------- //
size_t uint64_to_ascii_base10(uint64_t number, char * const output_buffer)
{
  char * const b = output_buffer; // rename variable to make code more readable.
  unsigned char d = 0;
//  unsigned char b[20]; // max of unsigned long long int is 20 digits long
  size_t l = 0;
  while(number > 0)
  {
    d = number % 10;
    number = number / 10;
    b[l] = 48 + d;
//    cout << "b[" << (unsigned int)(l) << "]='" << b[l] << "'("
//      << (unsigned int)(b[l]) << ")" << endl;
    l = l + 1;
  }
  for(unsigned char j=0; j<l/2; j++)
  {
//    cout << "swap:(" << (unsigned int)(j) << "," << (unsigned int)(l-j-1)
//      << ")" << endl;
//    cout << "b[" << (unsigned int)(j) << "]='" << b[j] << "'("
//      << (unsigned int)(b[j]) << ")" << endl;
//    cout << "b[" << (unsigned int)(l-j-1) << "]='" << b[l-j-1] << "'("
//      << (unsigned int)(b[l-j-1]) << ")" << endl;
    b[j] = b[j] + b[l-j-1];
//    cout << "b[" << (unsigned int)(j) << "]='" << b[j] << "'("
//      << (unsigned int)(b[j]) << ")" << endl;
    b[l-j-1] = b[j] - b[l-j-1];
//    cout << "b[" << (unsigned int)(l-j-1) << "]='" << b[l-j-1] << "'("
//      << (unsigned int)(b[l-j-1]) << ")" << endl;
    b[j] = b[j] - b[l-j-1];
//    cout << "b[" << (unsigned int)(j) << "]='" << b[j] << "'("
//      << (unsigned int)(b[j]) << ")" << endl;
//    cout << "b[" << (unsigned int)(l-j-1) << "]='" << b[l-j-1] << "'("
//      << (unsigned int)(b[l-j-1]) << ")" << endl;
  }
//  cout << "output=" << string((char*)(b), l) << endl;
  return l;
}
// -------------------------------------------------------------------------- //

