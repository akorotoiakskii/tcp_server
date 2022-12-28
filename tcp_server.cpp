#include <chrono>
#include <cstring>
#include <mutex>
#include "tcp_server.h"



//Parse ip to std::string
std::string getHostStr(uint32_t ip,uint16_t port) {
    return std::string() + std::to_string(int(reinterpret_cast<char*>(&ip)[0])) + '.' +
      std::to_string(int(reinterpret_cast<char*>(&ip)[1])) + '.' +
      std::to_string(int(reinterpret_cast<char*>(&ip)[2])) + '.' +
      std::to_string(int(reinterpret_cast<char*>(&ip)[3])) + ':' +
      std::to_string(port);
}


void TcpServer::add_manager(Manager * manager)
{
  manager_list.push_back(manager);
  max_length_command = std::max(manager->get_max_length_command(),
                                            max_length_command);
};

void TcpServer::set_window(MainWindow *w)
{
    m_window= w;
}


//if there is data 
void TcpServer::routing(){
    for(auto it = client_list.begin(), end = client_list.end(); it != end; ++it) {
        auto& client = *it;
        size_t command_length = client->command.length();
        size_t data_size = client->r_buffer->get_data_size();
        while(data_size){

            if(data_size>0)
                std::cout<<std::to_string(data_size)<<std::endl;

            char letter=client->r_buffer->read_char();
            //m_window->add_msg(std::string(1,letter));
            if(command_length<=max_length_command){
              if(letter!='\n'){
                client->command.push_back(letter);
                command_length++;
              }
              else{
                //if many managers

                for(auto & manager : manager_list){
                  std::string answer=manager->parser(client->command);
                  if(!answer.empty()){
                    client->sendData(answer+'\n');
                    break;//if find manager    break
                  }
                };
                m_window->add_msg(client->command);
                client->command="";
              }
            }
            else{
                command_length=0;
                client->command="";
            }
            data_size--;
            if(data_size==0)//last byte
                data_size = client->r_buffer->get_data_size();
        };
    };

    if(_status == SocketState::up){
      thread_pool.addJob([this](){routing();});
    }

}

TcpServer::TcpServer()
  :max_length_command(0)
    
{
  ka_conf = {};
}

TcpServer::~TcpServer() {
  if(_status == SocketState::up)
    stop();
}

uint16_t TcpServer::getPort() const 
{
  return port;
}
void TcpServer::set_port(uint16_t new_port)
{
  port =new_port;
}


SocketState TcpServer::start() {

  if(_status == SocketState::up) stop();

  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  address.sin_family = AF_INET;


  if((serv_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1){
    _status = SocketState::err_socket_init;
     return _status;
  }

  // Bind address to socket
   int flag = true;
  if((setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) ||
     (bind(serv_socket, (struct sockaddr*)&address, sizeof(address)) < 0)){
     _status = SocketState::err_socket_bind;
     return _status;
  }

  if(listen(serv_socket, SOMAXCONN) < 0){
    _status = SocketState::err_socket_listening;
    return _status;
  }
  _status = SocketState::up;
  thread_pool.addJob([this]{handlingAcceptLoop();});
  thread_pool.addJob([this]{waitingDataLoop();});
  thread_pool.addJob([this]{routing();});
  return _status;
}

void TcpServer::stop() {
  thread_pool.dropUnstartedJobs();
  _status = SocketState::close;
  close(serv_socket);
  client_list.clear();
}

void TcpServer::detach()
{
    thread_pool.detach();
}

void TcpServer::disconnectAll() {
  for(std::unique_ptr<Client>& client : client_list)
    client->disconnect();
}

void TcpServer::handlingAcceptLoop() {
  socklen_t addrlen = sizeof(sockaddr_in);
  sockaddr_in client_addr;
  Socket client_socket =  accept4(serv_socket,
               (struct sockaddr*)&client_addr, &addrlen, SOCK_NONBLOCK);
  if (client_socket >= 0 && _status == SocketState::up) {

    // Enable keep alive for client
    if(enableKeepAlive(client_socket)) {
      std::unique_ptr<Client> client(new Client(client_socket, client_addr));
      std::string connected_msg="Client connected - ";
      connected_msg.append(getHostStr(client->getHost(),client->getPort()));
      m_window->add_msg(connected_msg);
      client_mutex.lock();
      client_list.emplace_back(std::move(client));
      client_mutex.unlock();
    } else {
      shutdown(client_socket, 0);
      close(client_socket);
    }
  }

  if(_status == SocketState::up)
    thread_pool.addJob([this](){handlingAcceptLoop();});
}

void TcpServer::waitingDataLoop() {
  {
    std::lock_guard<std::mutex> lock(client_mutex);
    for(auto it = client_list.begin(), end = client_list.end(); it != end; ++it) {
      auto& client = *it;
      if(client){
        client->loadData();
        if(client->_status == SocketStatus::disconnected) {
          thread_pool.addJob([this, &client, it]{
            client->access_mtx.lock();
            Client* pointer = client.release();
            client = nullptr;
            pointer->access_mtx.unlock();
            client_list.erase(it);
            delete pointer;
          });
        }
      }
    }
  }

  if(_status == SocketState::up){
    thread_pool.addJob([this](){waitingDataLoop();});
  }
}

bool TcpServer::enableKeepAlive(Socket socket) {
  int flag = 1;
 //POSIX
  if(setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &flag,
                           sizeof(flag)) == -1)
    return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &ka_conf.ka_idle,
                   sizeof(ka_conf.ka_idle)) == -1)
    return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &ka_conf.ka_intvl,
                   sizeof(ka_conf.ka_intvl)) == -1)
    return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &ka_conf.ka_cnt,
                   sizeof(ka_conf.ka_cnt)) == -1)
    return false;
  return true;
}
