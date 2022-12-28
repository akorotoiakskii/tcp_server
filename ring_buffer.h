#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <mutex>
#include <vector>

class RingBuffer{

	size_t write_data_pos;
	size_t read_data_pos;
	char * main_buffer;
	size_t size_mask;
	size_t size_buff;

	std::mutex acces_mutex;
	bool is_exp_of_2(size_t size);	

	public:
	RingBuffer(size_t size);
	size_t get_data_size();
	void write_data(std::vector<char> &data_buf,size_t resv_size);
	void read_data(size_t size,std::vector<char> &data_buf);
	char read_char();
	~RingBuffer();
};

#endif // RINGBUFFER_H