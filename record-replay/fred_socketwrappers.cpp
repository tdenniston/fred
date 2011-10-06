/****************************************************************************
 * Copyright (C) 2009, 2010, 2011, 2012 by Kapil Arya, Gene Cooperman,      *
 *                                     Tyler Denniston, and Ana-Maria Visan *
 * {kapil,gene,tyler,amvisan}@ccs.neu.edu                                   *
 *                                                                          *
 * This file is part of FReD.                                               *
 *                                                                          *
 * FReD is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * FReD is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with FReD.  If not, see <http://www.gnu.org/licenses/>.            *
 ****************************************************************************/

#include "fred_wrappers.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
//#include "sockettable.h"
#include <pthread.h>
#include <sys/select.h>
#include <sys/un.h>
#include <arpa/inet.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "jassert.h"
#include "jfilesystem.h"
//#include "dmtcpworker.h"
#include "fred_wrappers.h"
#include "synchronizationlogging.h"

extern "C"
{
static __thread bool ok_to_log_sendrecv = false;

int socket ( int domain, int type, int protocol )
{
  BASIC_SYNC_WRAPPER(int, socket, _real_socket, domain, type, protocol);
}

int connect ( int sockfd,  const  struct sockaddr *serv_addr, socklen_t addrlen )
{
  BASIC_SYNC_WRAPPER(int, connect, _real_connect, sockfd, serv_addr, addrlen);
}

int bind ( int sockfd,  const struct  sockaddr  *addr,  socklen_t addrlen )
{
  BASIC_SYNC_WRAPPER(int, bind, _real_bind, sockfd, addr, addrlen);
}

int listen ( int sockfd, int backlog )
{
  BASIC_SYNC_WRAPPER(int, listen, _real_listen, sockfd, backlog);
}

int accept ( int sockfd, struct sockaddr *addr, socklen_t *addrlen )
{
  WRAPPER_HEADER(int, accept, _real_accept, sockfd, addr, addrlen);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(accept);
    if (retval != -1) {
      *addr = GET_FIELD(my_entry, accept, ret_addr);
      *addrlen = GET_FIELD(my_entry, accept, ret_addrlen);
    }
    WRAPPER_REPLAY_END(accept);
  } else if (SYNC_IS_RECORD) {
    isOptionalEvent = true;
    retval = _real_accept(sockfd, addr, addrlen);
    isOptionalEvent = false;
    if (retval != -1) {
      SET_FIELD2(my_entry, accept, ret_addr, *addr);
      SET_FIELD2(my_entry, accept, ret_addrlen, *addrlen);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)) && __GLIBC_PREREQ(2,10)
int accept4 ( int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags )
{
  WRAPPER_HEADER(int, accept4, _real_accept4, sockfd, addr, addrlen, flags);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(accept4);
    if (retval != -1) {
      *addr = GET_FIELD(my_entry, accept4, ret_addr);
      *addrlen = GET_FIELD(my_entry, accept4, ret_addrlen);
    }
    WRAPPER_REPLAY_END(accept4);
  } else if (SYNC_IS_RECORD) {
    retval = _real_accept4(sockfd, addr, addrlen, flags);
    if (retval != -1) {
      SET_FIELD2(my_entry, accept4, ret_addr, *addr);
      SET_FIELD2(my_entry, accept4, ret_addrlen, *addrlen);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}
//#endif

extern "C" int socketpair ( int domain, int type, int protocol, int sv[2] )
{
  void *return_addr = GET_RETURN_ADDRESS();
  if ((!shouldSynchronize(return_addr) && !log_all_socketpair) ||
      jalib::Filesystem::GetProgramName() == "gdb") {
    return _real_socketpair(domain, type, protocol, sv);
  };

  int retval;
  log_entry_t my_entry = create_socketpair_entry(my_clone_id, socketpair_event,
                                                 domain, type, protocol, sv);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(socketpair);
    if (retval != -1) {
      memcpy(sv, GET_FIELD(my_entry, socketpair, ret_sv), sizeof(sv));
    }
    WRAPPER_REPLAY_END(socketpair);
  } else if (SYNC_IS_RECORD) {
    retval = _real_socketpair(domain, type, protocol, sv);
    if (retval != -1) {
      memcpy(GET_FIELD(my_entry, socketpair, ret_sv), sv, sizeof(sv));
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

int setsockopt ( int sockfd, int  level,  int  optname,  const  void  *optval,
                 socklen_t optlen )
{
  WRAPPER_HEADER(int, setsockopt, _real_setsockopt,
                 sockfd, level, optname, optval, optlen);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(setsockopt);
  } else if (SYNC_IS_RECORD) {
    isOptionalEvent = true;
    retval = _real_setsockopt(sockfd, level, optname, optval, optlen);
    isOptionalEvent = false;
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

int getsockopt ( int sockfd, int  level,  int  optname,  void  *optval,
                 socklen_t *optlen )
{
  WRAPPER_HEADER(int, getsockopt, _real_getsockopt,
                 sockfd, level, optname, optval, optlen);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getsockopt);
    if (retval == 0 && optval != NULL) {
      *optlen = GET_FIELD(my_entry, getsockopt, ret_optlen);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getsockopt, optval, *optlen);
    }
    WRAPPER_REPLAY_END(getsockopt);
  } else if (SYNC_IS_RECORD) {
    isOptionalEvent = true;
    retval = _real_getsockopt(sockfd, level, optname, optval, optlen);
    isOptionalEvent = false;
    if (retval == 0 && optval != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getsockopt, optval, *optlen);
      SET_FIELD2(my_entry, getsockopt, ret_optlen, *optlen);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
  ok_to_log_sendrecv = true;
  ssize_t retval = sendto(sockfd, buf, len, flags, NULL, 0);
  ok_to_log_sendrecv = false;
  return retval;
}

extern "C" ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if ((!shouldSynchronize(return_addr) &&
       (SYNC_IS_NOOP || !ok_to_log_sendrecv)) ||
      jalib::Filesystem::GetProgramName() == "gdb") {
    return _real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
  }
  int retval;
  log_entry_t my_entry = create_sendto_entry(my_clone_id,
      sendto_event, sockfd, buf, len, flags, dest_addr, addrlen);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(ssize_t, sendto);
  } else if (SYNC_IS_RECORD) {
    WRAPPER_LOG(_real_sendto, sockfd, buf, len, flags, dest_addr, addrlen);
  }
  return retval;
}

extern "C" ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
  BASIC_SYNC_WRAPPER(ssize_t, sendmsg, _real_sendmsg, sockfd, msg, flags);
}

extern "C" ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
  ok_to_log_sendrecv = true;
  ssize_t retval = recvfrom(sockfd, buf, len, flags, NULL, NULL);
  ok_to_log_sendrecv = false;
  return retval;
}

extern "C" ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if ((!shouldSynchronize(return_addr) &&
       (SYNC_IS_NOOP || !ok_to_log_sendrecv)) ||
      jalib::Filesystem::GetProgramName() == "gdb") {
    return _real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
  }
  int retval;
  log_entry_t my_entry = create_recvfrom_entry(my_clone_id,
      recvfrom_event, sockfd, buf, len, flags, src_addr, addrlen);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(recvfrom);
    if (retval != -1) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(recvfrom, buf, retval);
      if (addrlen != NULL) {
        *addrlen = GET_FIELD(my_entry, recvfrom, ret_addrlen);
      }
    }
    WRAPPER_REPLAY_END(recvfrom);
  } else if (SYNC_IS_RECORD) {
    retval = _real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    if (retval != -1) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(recvfrom, buf, retval);
      if (addrlen != NULL) {
        SET_FIELD2(my_entry, recvfrom, ret_addrlen, *addrlen);
      }
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}


extern "C" ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
  WRAPPER_HEADER(ssize_t, recvmsg, _real_recvmsg, sockfd, msg, flags);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(recvmsg);
    if (retval != -1) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(recvmsg, msg, retval);
    }
    WRAPPER_REPLAY_END(recvmsg);
  } else if (SYNC_IS_RECORD) {
    retval = _real_recvmsg(sockfd, msg, flags);
    if (retval != -1) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(recvmsg, msg, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

}
