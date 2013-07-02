#ifndef BUFFER_H
#define BUFFER_H

// This must be a power of 2, i.e. 2^N, in order for this library to work
#define BUFF_SIZE 65536
#define BUFF_SIZE_MASK (BUFF_SIZE-1)

typedef struct buffer{
	//float buff[BUFF_SIZE];
	short *buff;
	unsigned int index;
}buffer;

void initBuffer(buffer* buffer);
void push(buffer* buffer,short value);
short pop(buffer* buffer);
short readn(buffer* buffer, int Xn);
void destroyBuffer(buffer* buffer);

#endif
