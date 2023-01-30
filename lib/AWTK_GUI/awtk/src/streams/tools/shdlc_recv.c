﻿// #ifndef WIN32_LEAN_AND_MEAN
// #define WIN32_LEAN_AND_MEAN 1
// #endif /*WIN32_LEAN_AND_MEAN*/

// #include "../../tkc/utils.h"
// #include "../../tkc/platform.h"
// #include "../inet/iostream_tcp.h"
// #include "../shdlc/iostream_shdlc.h"
// #include "../../tkc/socket_helper.h"

// void do_recv(int port)
// {
//   uint8_t buff[1024];
//   int slisten = tk_tcp_listen(port);
//   log_debug("listen: %d\n", port);
//   return_if_fail(slisten > 0);

//   do
//   {
//     int ret = 0;
//     int sock = tk_tcp_accept(slisten);
//     tk_iostream_t *tcp = tk_iostream_tcp_create(sock);
//     tk_iostream_t *shdlc = tk_iostream_shdlc_create(tcp);
//     tk_istream_t *is = tk_iostream_get_istream(shdlc);
//     tk_ostream_t *os = tk_iostream_get_ostream(shdlc);

//     memset(buff, 0x00, sizeof(buff));

//     do
//     {
//       ret = tk_istream_read(is, buff, sizeof(buff));
//       if (ret > 0)
//       {
//         log_debug("read: %s\n", (char *)buff);
//         ret = tk_ostream_write(os, buff, ret);
//       }
//       else
//       {
//         break;
//       }

//       if (!tk_object_get_prop_bool(TK_OBJECT(is), TK_STREAM_PROP_IS_OK, FALSE))
//       {
//         log_debug("client disconnected\n");
//         break;
//       }
//     } while (TRUE);

//     TK_OBJECT_UNREF(tcp);
//     TK_OBJECT_UNREF(shdlc);
//   } while (1);

//   return;
// }

// int main(int argc, char *argv[])
// {
//   int port = 0;

//   if (argc != 2)
//   {
//     printf("Usage: %s port\n", argv[0]);
//     return 0;
//   }

//   tk_socket_init();
//   platform_prepare();
//   TK_ENABLE_CONSOLE();

//   port = tk_atoi(argv[1]);
//   do_recv(port);

//   tk_socket_deinit();

//   return 0;
// }
