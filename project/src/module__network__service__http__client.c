// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module__network__service__http__client.h"
#include "module_terminal.h"
#include "module_heap.h"
#include "module_kernel.h"
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
#include "module__network__ip__tcp.h"
// -------------------------------------------------------------------------- //
bool module__network__service__http__client__request_response(
  module__network__ethernet_interface * const interface,
  const uint32_t server_ip, const module__network__data__mac_address server_mac,
  const uint16_t server_port)
{
  // See: https://packetlife.net/blog/2010/jun/7/
  // understanding-tcp-sequence-acknowledgment-numbers/

  module_terminal_global_print_c_string("\n=============\n");

  const uint16_t random_port = 54321;
  module__network__ip__tcp__session s;
  module__network__ip__tcp__session_create(&s, interface->mac_address,
    interface->ip, random_port, server_mac, server_ip, server_port);

  // step 1 = connect
  module__network__ip__tcp__connect(interface, &s);

  // step 2 = send request
  const char * const request =
    "GET / HTTP/1.1\r\n"
    "User-Agent: Wget/1.20.1 (linux-gnu)\r\n"
    "Accept: */*\r\n"
    "Accept-Encoding: identity\r\n"
    "Host: 10.0.2.2:1030\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n"
    ;
  const size_t request_size = 140;
  uint8_t r = 1;
  r = module__network__ip__tcp__send(interface, &s, request, request_size);
  if(r != 0)
  {
    module_terminal_global_print_c_string("error while sending data to TCP"
      " server! err=");
    module_terminal_global_print_uint64(r);
    module_terminal_global_print_c_string("\n");
    return false;
  }

  // step 3 = receive response
  const size_t buffer_size = 1024*1024;
  char buffer[1024*1024];
  size_t bytes_read = 0;
  size_t content_length = 0;
  size_t response_offset = 0;
  { // process HTTP header
    size_t i = 0;
    bool is_key = true; // true = key, false = value
    bool is_http_header_complete = false;
    char * key_start = NULL;
    size_t key_size = 0;
    char * value_start = NULL;
    size_t value_size = 0;
    while(!is_http_header_complete)
    {
      module__network__ip__tcp__recv(interface, &s, buffer, buffer_size,
        &bytes_read);
      i = 0;

      // skip first line
      while(i < bytes_read)
      {
        if(buffer[i] == '\r' && (i+1 < bytes_read) && buffer[i+1] == '\n')
        {
          i += 2; // skip \r and \n too
          break;
        }
        i++; // skip this character that was processed
      }

      // process key-value pairs (header)
      is_key = true;
      key_start = buffer + i;
      key_size = 0;
      while(i < bytes_read)
      {
        if(buffer[i] == '\r')
        {
          if((i+1 < bytes_read) && buffer[i+1] == '\n')
          {
            if(key_size == 0)
            {
              is_http_header_complete = true;
              response_offset = i + 2; // overflow ?
              break;
            }
            else
            {
              // skip first character(space) in value
              value_start++;
              value_size--;

//              module_terminal_global_print_buffer_bytes((uint8_t*)key_start,
//                key_size);
//              module_terminal_global_print_c_string(": ");
//              module_terminal_global_print_buffer_bytes(
//                (uint8_t*)(value_start), value_size);
//              module_terminal_global_print_c_string("\n");

              if(module_kernel_memcmp(key_start, "Content-Length", 14) == 0)
              {
//                module_terminal_global_print_c_string("CL_str=");
//                module_terminal_global_print_buffer_bytes(
//                  (uint8_t*)(value_start), value_size);
//                module_terminal_global_print_c_string("\n");
                content_length = 0;
                // ATOI begin
                for (size_t j=value_size-1; ; j--)
                {
                  size_t coef = 1;
                  for (size_t k=0; k<j; k++)
                  {
                    coef *= 10;
                  }
                  content_length += coef * ((uint8_t)(value_start[j]) - 48);
                  if(j == 0)
                  {
                    break;
                  }
                }
                // ATOI end
//                module_terminal_global_print_c_string("CL=");
//                module_terminal_global_print_uint64(content_length);
//                module_terminal_global_print_c_string("\n");
              }

              i += 2; // skip \n too
              is_key = true;
              key_start = buffer + i;
              key_size = 0;
              continue;
            }
          }
          // else - consider it part of key or value
        }
        else if(buffer[i] == ':')
        {
          if(is_key)
          {
            is_key =false;
            i++;
            value_start = buffer + i;
            value_size = 0;
            continue;
          }
          //else - if ':' is part of the value, it's ok
        }
        // else - consider it part of key or value
        i++; // skip this character that was processed
        if(is_key)
        {
          key_size++;
        }
        else
        {
          value_size++;
        }
      }
    } // end while ! http header complete
  } // end process header

  // read content
  char* response = (char*)malloc(content_length);
  module_kernel_memcpy(buffer + response_offset, response,
    bytes_read - response_offset);
  bytes_read -= response_offset;
  while(content_length != bytes_read)
  {
    module__network__ip__tcp__recv(interface, &s, response + bytes_read,
      content_length, &bytes_read);
//    module_terminal_global_print_c_string("BR=");
//    module_terminal_global_print_uint64(bytes_read);
//    module_terminal_global_print_c_string("\n");
  }
  module_terminal_global_print_c_string("\n=====FIRST_100========\n");
  module_terminal_global_print_buffer_bytes((uint8_t*)(response), 100);
  module_terminal_global_print_c_string("\n======LAST_100========\n");
  module_terminal_global_print_buffer_bytes(
    (uint8_t*)(response+content_length-100), 100);
  module_terminal_global_print_c_string("\n=============\n");

  bool is_ok = false;
  return is_ok;
}
// -------------------------------------------------------------------------- //
