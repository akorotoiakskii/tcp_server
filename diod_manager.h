#ifndef DIOD_MAGAER_H
#define DIOD_MAGAER_H


#include "manager.h"
#include "mainwindow.h"

struct Command{
	std::string command;
    std::list<std::string > args_set;//defined args
    std::string regex_check;//undefined args
    int max_length_regex;
};


class DiodManager: Manager {

	std::list<Command> list_command;
	void create_list_command();
	void setInterval(int interval);
	
	std::list<std::string> command_start_words;
	size_t max_length_command;

	bool status_diod;
	int rate_diod;
	std::string color_diod;
    bool  * stop_thread;
	bool current_state;
    MainWindow *m_window;
    void routine();

	std::string set_led_state(std::string arg);
	std::string set_led_color(std::string arg);
	std::string set_led_rate(std::string arg);


	std::string get_led_state();
	std::string get_led_color();
	std::string get_led_rate();

	public:
    void start();
    void stop();
    void set_window(MainWindow *w);
	std::string parser(std::string &data) override;
	size_t get_max_length_command() override;
	DiodManager();
};



#endif //DIOD_MAGAER_H
