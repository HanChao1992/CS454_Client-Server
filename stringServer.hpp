#include <string>

using namespace std;

class stringServer {
  public:
    stringServer();
    int serverSetup();
    void printInfo(int sockfd);
    void operateConn(int sockfd, fd_set &master, fd_set &read_fds, int &fdmax);
    void processConn(int newfd, fd_set &master);
    void closeConn(int sockfd);
  private:
    void stringConvert(string &s);
};
