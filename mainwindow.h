#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void set_red_color();
    void set_green_color();
    void set_blue_color();
    void set_ON_diod();
    void set_OFF_diod();
    void add_msg(std::string msg);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
