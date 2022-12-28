#include "ring_buffer.h"
#include <new>
#include <iostream>
#include <cstring>





bool RingBuffer::is_exp_of_2(size_t size)
{
	return ( size & (size - 1) ) == 0;
};

RingBuffer::RingBuffer(size_t size)
  : write_data_pos(0),
    read_data_pos(0),
    main_buffer(nullptr),
    size_mask(0),
    size_buff(0)
{
	if(is_exp_of_2(size)){
		try {
			main_buffer =new char[size];
		} catch (const std::bad_alloc& e) {
			std::cout << "RingBuffer main_buffer allocation failed: "
										<< e.what() << std::endl;

		}
		size_mask=size-1;
		size_buff = size;
	}
	else
		std::cout << "size of  RingBuffer is error: " << std::endl;
};

void RingBuffer::write_data(std::vector<char> &data_buf,size_t resv_size)
{
	size_t data_buf_size = resv_size;
	size_t current_write_pos= write_data_pos&size_mask;

	if((current_write_pos+data_buf_size)<= size_buff){
		std::memcpy(main_buffer+current_write_pos,
							data_buf.data(), data_buf_size);
	}
	else{
		std::memcpy(main_buffer+current_write_pos, 
					data_buf.data(),(size_buff-current_write_pos));

		std::memcpy(main_buffer, 
					data_buf.data()+(size_buff-current_write_pos),
					data_buf_size-(size_buff-current_write_pos));
	}
	acces_mutex.lock();
	write_data_pos+=data_buf_size;
	acces_mutex.unlock();
}

void RingBuffer::read_data(size_t size,std::vector<char> &data_buf)
{
	if(data_buf.size()!=size){
		data_buf.resize(size);
	}
	size_t data_buf_size = data_buf.size();
	size_t current_read_pos= read_data_pos&size_mask;

	if((current_read_pos+data_buf_size)<= size_buff){
		std::memcpy(data_buf.data(), 
					main_buffer+current_read_pos,
					data_buf_size);
	}
	else{
		std::memcpy(data_buf.data(),
					main_buffer+current_read_pos, 
					(size_buff-current_read_pos));

		std::memcpy(data_buf.data()+(size_buff-current_read_pos),
					main_buffer, 
					data_buf_size-(size_buff-current_read_pos));
	}
    acces_mutex.lock();
	read_data_pos+=data_buf_size;
    acces_mutex.unlock();

};


char RingBuffer::read_char()
{
	char data = main_buffer[read_data_pos&size_mask];
    acces_mutex.lock();
	read_data_pos++;
    acces_mutex.unlock();
	return data;

};

size_t RingBuffer::get_data_size()
{
	size_t data_size=0;

	acces_mutex.lock();
	data_size = write_data_pos - read_data_pos;
	acces_mutex.unlock();

	data_size&=size_mask;

	return data_size;
};

RingBuffer::~RingBuffer(){
	if(main_buffer!=nullptr)
		delete[] main_buffer;
};
