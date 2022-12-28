#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <cstdint>
#include <list>
#include <string>

class Manager{
	public:
	virtual std::string parser(std::string &data)= 0;
	virtual size_t get_max_length_command()= 0;

};



#endif // MANAGER_H