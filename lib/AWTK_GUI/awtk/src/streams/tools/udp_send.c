﻿// #ifndef WIN32_LEAN_AND_MEAN
// #define WIN32_LEAN_AND_MEAN 1
// #endif /*WIN32_LEAN_AND_MEAN*/

// #include "../../tkc/utils.h"
// #include "../../tkc/platform.h"
// #include "../inet/iostream_udp.h"
// #include "../../tkc/socket_helper.h"

// void do_send(tk_iostream_t *iostream, const char *msg)
// {
//   char buff[1024];
//   int32_t ret = 0;
//   uint32_t size = strlen(msg) + 1;
//   tk_istream_t *istream = tk_iostream_get_istream(iostream);
//   tk_ostream_t *ostream = tk_iostream_get_ostream(iostream);

//   memset(buff, 0x00, sizeof(buff));
//   ret = tk_ostream_write(ostream, (uint8_t *)msg, size);
//   log_debug("send ret=%d %s\n", ret, msg);

//   ret = tk_istream_read(istream, (uint8_t *)buff, sizeof(buff));
//   log_debug("%d: %s\n", ret, buff);

//   tk_object_unref(TK_OBJECT(iostream));

//   return;
// }

// int main(int argc, char *argv[])
// {
//   int port = 0;
//   int local_port = -1;
//   const char *msg = NULL;
//   const char *host = NULL;
//   const char *local_ip = NULL;

//   if (argc != 4 && argc != 6)
//   {
//     printf("Usage: %s host port msg\n", argv[0]);
//     printf("       %s host port local_ip local_port msg\n", argv[0]);
//     return 0;
//   }

//   tk_socket_init();
//   platform_prepare();
//   TK_ENABLE_CONSOLE();

//   host = argv[1];
//   port = tk_atoi(argv[2]);
//   if (argc == 4)
//   {
//     msg = argv[3];
//   }
//   else
//   {
//     local_ip = argv[3];
//     local_port = tk_atoi(argv[4]);
//     msg = argv[5];
//   }

//   do_send(tk_iostream_udp_create_client_ex(host, port, local_ip, local_port), msg);

//   tk_socket_deinit();

//   return 0;
// }
