#include "tcp_server.h"
#include <iostream>

bool Client::loadData() {

  if(_status != SocketStatus::connected) 
    return false;
  
  int err;

  int answ = recv(socket, buffer.data(), size_buff, MSG_DONTWAIT);
  if(answ>0){
    r_buffer->write_data(buffer,answ);
    std::cout<<std::string(buffer.data())<<std::endl;
  }

  // Disconnect
  if(!answ) {
    disconnect();
    return false;
  } else if(answ == -1) {
    socklen_t len = sizeof (err);
	  getsockopt (socket, SOL_SOCKET, SO_ERROR, &err, &len);
    if(!err) 
		err = errno;
    switch (err) {
      case 0: break;
        // Keep alive timeout
      case ETIMEDOUT:
      case ECONNRESET:
      case EPIPE:
        disconnect();
        // No data
      case EAGAIN: 
        return false;
      default:
        disconnect();
        std::cerr << "Unhandled error!\n"
                    << "Code: " << err << " Err: " << std::strerror(err) << '\n';
      return false;
    }
  }
  if(answ>0)
    return true;
  else
    return false;
}


SocketStatus Client::disconnect() {
  	_status = SocketStatus::disconnected;
	if(socket == -1)
  		return _status;
  	shutdown(socket, 0);
  	close(socket);
  	socket = -1;
  	return _status;
}

bool Client::sendData(const std::string &command) const {
  	if(_status != SocketStatus::connected) 
   		return false;

 	if(send(socket, command.data(), command.length(), 0) < 0) 
  		return false;
	return true;
}

Client::Client(Socket socket, sockaddr_in address)
  : address(address),
   socket(socket),
   size_buff(64*1024)
{
   r_buffer = new RingBuffer(2*1024*1024);
   _status = SocketStatus::connected;
   buffer.resize(size_buff);
}


Client::~Client() {
	delete r_buffer;
  	if(socket == -1) 
  		return;
  	shutdown(socket, 0);
  	close(socket);
}

SocketStatus Client::getStatus() const  
{return _status;}

uint32_t Client::getHost() const {return address.sin_addr.s_addr;}
uint16_t Client::getPort() const {return address.sin_port;}
