#include "buffer.h"

void initBuffer(buffer* buffer){
	int i;
	buffer->index = 0;
	for(i=0;i<BUFF_SIZE;i++){
		buffer->buff[i]=0;
	}
}

void push(buffer* buffer,float value){
	buffer->buff[(buffer->index++) & BUFF_SIZE_MASK] = value;
}

float pop(buffer* buffer){
	return buffer->buff[(buffer->index + (~BUFF_SIZE_MASK)) & BUFF_SIZE_MASK];
}

float readn(buffer* buffer, int Xn){
	return buffer->buff[(buffer->index + (~Xn)) & BUFF_SIZE_MASK];
}
