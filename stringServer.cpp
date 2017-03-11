#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "stringServer.hpp"

using namespace std;
const int maxConn = 5; // as specified in A2

stringServer::stringServer() {
}

void stringServer::printInfo(int sockfd) {
  char hname[512];
  gethostname(hname, 512);
  cout << "SERVER_ADDRESS " << hname << endl;
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  getsockname(sockfd, (struct sockaddr *)&sin, &len);
  cout << "SERVER_PORT " << ntohs(sin.sin_port) << endl;
}

int stringServer::serverSetup() {
  int status;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, "0", &hints, &res);
  if(status) {
    cerr << "getaddrinfo error: "<< gai_strerror(status) << "." << endl;
    return 0;
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  listen(sockfd, maxConn);
  return sockfd;
}

void stringServer::operateConn(int sockfd, fd_set &master, fd_set &read_fds, int &fdmax) {
  int status, newfd;
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  read_fds = master;
  status = select(fdmax+1, &read_fds, NULL, NULL, NULL);
  if (status == -1) {
    cerr << "Unable to select..." << endl;
  }
  for (int i = 0; i <= fdmax; i++) {
    if(FD_ISSET(i, &read_fds)) {
      if (i == sockfd) {
        addr_size = sizeof their_addr;
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (newfd == -1) {
          cerr << "Unable to accept socket " << sockfd << "." << endl;
          close(sockfd);
          continue;
        }
        else {
          FD_SET(newfd, &master);
          if (newfd > fdmax) {
            fdmax = newfd;
          }
        }
      }
      else {
        processConn(i, master);
      }
    }
    //closeConn(i);
  }
}

void stringServer::processConn(int newfd, fd_set &master) {
  //cout << "Processing..." << endl;
  int len, status;
  status = recv(newfd, &len, sizeof len, 0);
  if (status == -1) {
    cerr << "Failed to recv string length" << endl;
    return;
  }
  if (status == 0) {
    cerr << "Client closed connection.." << endl;
    FD_CLR(newfd, &master);
    return;
  }
  //cout << "recv len done" << endl;
  char* s = new char [len];
  status = recv(newfd, s, len, 0);
  if (status == -1) {
    cerr << "Failed to recv string" << endl;
    return;
  }
  string sr(s);
  stringConvert(sr);
  const char* sendS = sr.c_str();
  len = sr.length() + 1;
  status = send(newfd, &len, sizeof len, 0);
  if (status == -1) {
    cerr << "Unable to send length of the string." << endl;
    return;
  }

  status = send(newfd, sendS, len, 0);
  if (status == -1) {
    cerr << "Unable to send string." << endl;
    return;
  }
}

void stringServer::closeConn(int sockfd) {
  close(sockfd);
}

void stringServer::stringConvert(string &s) {
  bool isSpace = true;
  int len = s.length();
  for (int i = 0; i < len; i++) {
    if (s[i] == ' ' || s[i] == '\t') {
      isSpace = true;
      continue;
    }
    if (isSpace) {
      s[i] = toupper(s[i]);
    }
    else {
      s[i] = tolower(s[i]);
    }
    isSpace = false;
  }
}


int main() {
  stringServer sServer;
  int sock;
  fd_set master;
  fd_set read_fds;
  int fdmax;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  sock = sServer.serverSetup();
  FD_SET(sock, &master);
  fdmax = sock;
  sServer.printInfo(sock);
  while (true) {
    sServer.operateConn(sock, master, read_fds, fdmax);
  }
}
