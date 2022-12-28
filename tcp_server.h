#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <functional>
#include <list>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "general.h"
#include "manager.h"
#include "ring_buffer.h"
#include "mainwindow.h"

struct KeepAliveConfig{
  ka_prop_t ka_idle = 120;
  ka_prop_t ka_intvl = 3;
  ka_prop_t ka_cnt = 5;
};


class Client {

  typedef SocketStatus status;

  sockaddr_in address;
  Socket socket;
  std::vector<char>  buffer;
  const size_t size_buff;

public:
  Client(Socket socket, sockaddr_in address);
  ~Client() ;
  uint32_t getHost() const;
  uint16_t getPort() const;
  SocketStatus _status;
  status getStatus() const;
  SocketStatus disconnect() ;

  bool loadData();
  std::mutex access_mtx;
  bool sendData(const std::string &command) const;
  SocketType getType() const {return SocketType::server_socket;}


  RingBuffer * r_buffer;
  std::string command;

};




class TcpServer {

  Socket serv_socket;
  uint16_t port;
  ThreadPool thread_pool;
  KeepAliveConfig ka_conf;
  std::list<std::unique_ptr<Client>> client_list;
  std::mutex client_mutex;

  bool enableKeepAlive(Socket socket);
  void handlingAcceptLoop();
  void waitingDataLoop();
  void routing();

  std::list<Manager *> manager_list;
  size_t max_length_command;
  MainWindow *m_window;
public:


  SocketState _status = SocketState::close;

  void set_window(MainWindow *w);
  TcpServer();
  ~TcpServer();


  void add_manager(Manager * manager);

  // Server properties getters
  uint16_t getPort() const;
  void set_port(uint16_t new_port);
  SocketState getStatus() const {return _status;}

  // Server status manip
  SocketState start();
  void stop();
  void detach();

  // Server client management
  void disconnectAll();
};


#endif // TCP_SERVER_H
