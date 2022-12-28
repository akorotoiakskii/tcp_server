#include "diod_manager.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <functional>


DiodManager::DiodManager()
{
    rate_diod=1;
	max_length_command=0;
	current_state=false;
    stop_thread=0;
}

void DiodManager::start()
{
    set_led_state("on");
    set_led_color("red");
    create_list_command();
    setInterval(rate_diod*500);//1000  -period,duty cycle - 50%
}

void DiodManager::routine(){
	if(status_diod){
		current_state=!current_state;
        if(current_state)
            m_window->set_ON_diod();
        else
            m_window->set_OFF_diod();
    }
    else{
		current_state=false;
        m_window->set_OFF_diod();
    }
}


void DiodManager::set_window(MainWindow *w)
{
    m_window= w;
}


void DiodManager::setInterval(int interval){

    if(interval!=0){
        if(stop_thread!=0)
            *stop_thread = true;
        stop_thread=  new bool();
        *stop_thread = false;
        std::thread t = std::thread([&,interval]() {
            while(true) {
                if(*stop_thread){
                    delete stop_thread;
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(*stop_thread){
                    delete stop_thread;
                    return;
                }
                routine();
            }
        });
        t.detach();
        return;
    }
    else{//kill thread if interval==0
        if(stop_thread!=0)
            *stop_thread = true;
    }
}


void DiodManager::stop()
{
    *stop_thread = true;
}


std::string DiodManager::set_led_state(std::string arg){

	std::string answer;
	if(arg=="on"){
		status_diod=true;
        m_window->set_ON_diod();
		answer = "OK";
	}
	else if(arg=="off"){
		status_diod=false;
        m_window->set_OFF_diod();
		answer = "OK";
	}
	else
		answer = "FAILED";

	return answer;
};

std::string DiodManager::set_led_color(std::string arg){
	std::string answer;
	if(arg=="red"){
		color_diod="red";
        m_window->set_red_color();
		answer = "OK";
	}
	else if(arg=="green"){
		color_diod="green";
        m_window->set_green_color();
		answer = "OK";
	}
	else if(arg=="blue"){
		color_diod="blue";
        m_window->set_blue_color();
		answer = "OK";
	}
	else
		answer = "FAILED";

	return answer;

};

std::string DiodManager::set_led_rate(std::string arg){

	std::string answer;
	int rate=0;
	try{
		rate = std::stoi(arg);
		if(rate>=0 && rate<=5){
			rate_diod = rate;
            setInterval(rate_diod*500);//1000  -period,duty cycle - 50%
			answer = "OK";
		}
		else
			answer = "FAILED";
	}
	catch(std::invalid_argument const& ex){
		answer = "FAILED";
	}
	catch(std::out_of_range const& ex){
		answer = "FAILED";
	}
	return answer;
};

std::string DiodManager::get_led_state(){
	std::string answer;
	answer =answer + "OK" + " ";

	if(status_diod)
		answer+="on";
	else
		answer+="off";
	return answer;
};

std::string DiodManager::get_led_color(){
	std::string answer;
	answer =answer + "OK" + " ";
	
	answer+=color_diod;
	return answer;

};

std::string DiodManager::get_led_rate(){
	std::string answer;
	answer =answer + "OK" + " ";
	
	answer+=std::to_string(rate_diod);
	return answer;
};

void DiodManager::create_list_command()
{
    std::string str_emty;
    list_command.push_back(Command{"set-led-state",{"on","off"},str_emty,0});
    list_command.push_back(Command{"get-led-state",{},"",0});
	list_command.push_back(Command{"set-led-color",
                                        {"red","green","blue"},"",0 });
    list_command.push_back(Command{"get-led-color",{},"",0});
	list_command.push_back(Command{"set-led-rate",{},
										"^[0-5]$",1});// from 0 to 5
    list_command.push_back(Command{"get-led-rate",{},"",0});


	for(auto &command : list_command){
		command_start_words.push_back(command.command);

		size_t command_all_length=command.command.length();

		if(!command.args_set.empty()){
			size_t max_len_arg=0;
			for(auto &arg : command.args_set){
				max_len_arg=std::max(max_len_arg,arg.size());
			}
			command_all_length+=max_len_arg;
		}

		if(!command.regex_check.empty())
			command_all_length+=command.max_length_regex;
								
		max_length_command=std::max(max_length_command,
									command_all_length);
	};

	max_length_command+=1;//space between
}

//return "" if not fined command
std::string DiodManager::parser(std::string &data){
	std::string answer;

	size_t pos_space =data.find(" ");
	//set function
	if(pos_space!=std::string::npos && pos_space!=0){

        std::string command=data.substr(0,pos_space);

        std::string arg=data.substr(pos_space+1,data.size()-1);
		
		if (command==std::string("set-led-state"))
			answer=set_led_state(arg);
		else if (command==std::string("set-led-color"))
			answer=set_led_color(arg);
		else if (command==std::string("set-led-rate"))
			answer=set_led_rate(arg);

	}
	else{// get function
		if (data==std::string("get-led-state"))
			answer=get_led_state();
		else if (data==std::string("get-led-color"))
			answer=get_led_color();
		else if (data==std::string("get-led-rate"))
			answer=get_led_rate();
	}

	return answer;
};

size_t DiodManager::get_max_length_command(){
	return max_length_command;
};

