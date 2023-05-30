#include "svrkit_test/my_svrkit/log/log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char *request = "p";

int add_fd(int epoll_fd, int fd) {
  struct epoll_event event;
  event.events = EPOLLOUT | EPOLLET | EPOLLERR | EPOLLHUP;
  event.data.fd = fd;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

bool write_nbytes(int sockfd, const char *buffer, uint32_t len) {
  int bytes_write = 0;

  PLOG_RUN("write length[%u] buffer[%s] to fd[%d]", len, buffer, sockfd);
  while (true) {
    bytes_write = send(sockfd, buffer, len, 0);
    if (bytes_write == -1) {
      PLOG_ERR("send failed", bytes_write, "errno[%d], error[%s]", errno, strerror(errno));
      return false;
    }
    if (bytes_write == 0) {
      PLOG_RUN("send 0 bytes");
      return false;
    }
    PLOG_RUN("send length[%d] bytes to fd[%d]", bytes_write, sockfd);

    len -= bytes_write;
    buffer = buffer + bytes_write;
    if (len <= 0) {
      PLOG_RUN("send complete to fd[%d]", sockfd);
      return true;
    }
  }
}

bool read_once(int sockfd, char *buffer, int len) {
  int bytes_read = 0;
  memset(buffer, '\0', len);
  bytes_read = recv(sockfd, buffer, len, 0);
  if (bytes_read == -1) {
    PLOG_ERR("recv failed", bytes_read, "errno[%d], error[%s]", errno, strerror(errno));
    return false;
  }
  if (bytes_read == 0) {
    PLOG_RUN("recv 0 bytes");
    return false;
  }
  PLOG_RUN("read in %d bytes buffer[%s] from socket %d", bytes_read, buffer, sockfd);

  return true;
}

int start_conn(int epoll_fd, const char *ip, int port, uint32_t num) {
  uint32_t num_of_succ_conn = 0;
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);

  for (uint32_t i = 0; i < num; ++i) {
    int ret =  0;
    // TODO: 非阻塞式
    ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ret < 0) {
      PLOG_ERR("socket failed", ret, "errno[%d], error[%s]", errno, strerror(errno));
      continue;
    }
    int fd = ret;

    ret = connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (ret < 0) {
      PLOG_ERR("connect failed", ret, "errno[%d], error[%s]", errno, strerror(errno));
      continue;
    }

    ret = add_fd(epoll_fd, fd);
    if (ret < 0) {
      PLOG_ERR("add_fd err", ret, "errno[%d] error[%s]", errno, strerror(errno));
      close(fd);
      continue;
    }

    ++num_of_succ_conn;
    PLOG_RUN("build connection %d", num_of_succ_conn);
  }

  return num_of_succ_conn;
}

int close_conn(int epoll_fd, int fd)
{
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    if (ret < 0) {
      PLOG_ERR("epoll del err", ret, "epfd[%d] fd[%d] errno[%d] errmsg[%s]", epoll_fd, fd, errno, strerror(errno));
      return ret;
    }
    PLOG_RUN("epoll del succ epfd[%d] sockfd[%d]", epoll_fd, fd);

    ret = close(fd);
    if (ret < 0) {
      PLOG_ERR("close err", ret, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
      return ret;
    }

    PLOG_RUN("close fd[%d]", fd);
    return 0;
}

int main(int argc, char **argv) {
  if (argc != 4) {
      printf("usage:stress_test ip port num\n");
      return -1;
  }

  int epoll_fd = epoll_create1(0);
  uint32_t num = static_cast<uint32_t>(atoi(argv[3]));
  int port = atoi(argv[2]);
  uint32_t num_of_succ_conn = start_conn(epoll_fd, argv[1], port, num);
  epoll_event events[10000];
  char buffer[2048];
  while (num_of_succ_conn > 0) {
    int fds = epoll_wait(epoll_fd, events, 10000, 2000);
    for (int i = 0; i < fds; ++i) {
      int sockfd = events[i].data.fd;
      if (events[i].events & EPOLLIN) {
        if (!read_once(sockfd, buffer, 2048)) {
          PLOG_RUN("close fd[%d] after read error", sockfd);
          num_of_succ_conn -= close_conn(epoll_fd, sockfd) >= 0 ? 1 : 0;
          PLOG_RUN("num_of_succ_conn[%u]", num_of_succ_conn);
          continue;
        }
        num_of_succ_conn -= close_conn(epoll_fd, sockfd) >= 0 ? 1 : 0;
        PLOG_RUN("num_of_succ_conn[%u]", num_of_succ_conn);
        PLOG_RUN("close conn fd[%d] for end", sockfd);
        continue;
      }
      if (events[i].events & EPOLLOUT) {
        if (!write_nbytes(sockfd, request, strlen(request))) {
          PLOG_RUN("close fd[%d] after write error", sockfd);
          num_of_succ_conn -= close_conn(epoll_fd, sockfd) >= 0 ? 1 : 0;
          PLOG_RUN("num_of_succ_conn[%u]", num_of_succ_conn);
          continue;
        }

        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
        event.data.fd = sockfd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
        PLOG_RUN("modify fd[%d] to in", sockfd);
        continue;
      }
      if (events[i].events & EPOLLHUP) {
        close_conn(epoll_fd, sockfd);
      }
    }
  }

  return 0;
}
