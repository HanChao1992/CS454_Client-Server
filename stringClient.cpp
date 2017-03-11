#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

using namespace std;

struct thread_data {
  int sockfd;
  string s;
};

void *sendRecv(void *threadarg) {
  int status;
  struct thread_data *my_data;
  my_data = (struct thread_data *) threadarg;
  //cout << "Thread created for " << my_data->s << endl;
  int len = my_data->s.length() + 1;
  status = send(my_data->sockfd, &len, sizeof(len), 0);
  if (status == -1) {
    cerr << "Error sending string length for string: \"" << my_data->s
    << "\".";
    exit(0);
  }
  status = send(my_data->sockfd, my_data->s.c_str(), len, 0);
  if (status == -1) {
    cerr << "Error sending string: \"" << my_data->s
    << "\".";
    exit(0);
  }
  char *recvStr = new char[len];
  status = recv(my_data->sockfd, &len, sizeof(len), 0);
  if (status == -1) {
    cerr << "Error receiving length of title case string for string: \"" << my_data->s
    << "\".";
    exit(0);
  }
  status = recv(my_data->sockfd, recvStr, len, 0);
  if (status == -1) {
    cerr << "Error receiving title case string for string: \"" << my_data->s
    << "\".";
    exit(0);
  }
  cout << "Server: " << recvStr << endl;
  sleep(2);
  pthread_exit(NULL);
}

int main() {
  int status;
  char *serverAddress = getenv("SERVER_ADDRESS");
  char *port = getenv("SERVER_PORT");
  if (serverAddress == NULL) {
    cerr << "Missing SERVER_ADDRESS" << endl;
    exit(0);
  }
  if (port == NULL) {
    cerr << "Missing SERVER_PORT" << endl;
    exit(0);
  }
  int sockfd, portno;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  portno = atoi(port);
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  server = gethostbyname(serverAddress);

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  status = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (status < 0) {
    cerr << "Failed to connect" << endl;
    exit(0);
  }
  struct thread_data td;
  td.sockfd = sockfd;
  while(true) {
    string str;
    getline(cin, str);
    td.s = str;
    pthread_t thread;
    pthread_create(&thread, NULL, sendRecv, (void *)&td);
  }
}
