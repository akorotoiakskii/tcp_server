


#include <iostream>
#include <cstdlib>
#include "tcp_server.h"
#include "diod_manager.h"


#include "mainwindow.h"
#include <QApplication>



TcpServer server;
DiodManager diod_manager;

int main(int argc, char *argv[])
{
    int port = 20001;
    if(port>65535)
        return EXIT_FAILURE;


    QApplication a(argc,argv);
    MainWindow w;
    w.show();


    diod_manager.set_window(&w);
    diod_manager.start();
    server.set_port(port);
    server.add_manager((Manager *)&diod_manager);
    server.set_window(&w);


    try {
        //Start server
        if(server.start() == SocketState::up) {
            std::string listening_msg="Server listen on port: ";
             listening_msg.append(std::to_string(server.getPort()));
             w.add_msg(listening_msg);
            server.detach();
            //join
            a.exec();
            server.stop();
            diod_manager.stop();

            return EXIT_SUCCESS;
        } else {
            std::cerr<<"Server start error! Error code:"
                            << int(server.getStatus()) <<std::endl;
            return EXIT_FAILURE;
        }
    }
    catch(std::exception& except) {
        std::cerr << except.what();
        return EXIT_FAILURE;
    }

}
