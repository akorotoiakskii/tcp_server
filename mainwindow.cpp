#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    set_red_color();
    set_OFF_diod();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_red_color(){

    ui->diod_label->setStyleSheet("background-color:red;");
    ui->diod_label->show();
}

void MainWindow::set_green_color(){

    ui->diod_label->setStyleSheet("background-color:green;");
    ui->diod_label->show();
}

void MainWindow::set_blue_color(){

    ui->diod_label->setStyleSheet("background-color:blue;");
    ui->diod_label->show();
}

void MainWindow::set_ON_diod(){

    ui->diod_label->show();
}

void MainWindow::set_OFF_diod(){
    ui->diod_label->hide();
}


void MainWindow::add_msg(std::string msg){
    static std::list<std::string> msgs;
    msgs.push_back(msg);

    if(msgs.size()>10)
        msgs.pop_front();

    std::string str_out;
    for (auto &str:msgs){
        str_out.append(str);
        str_out.append("\n");
    }

    ui->last_msg_label->setText(QString::fromStdString(str_out));
}

