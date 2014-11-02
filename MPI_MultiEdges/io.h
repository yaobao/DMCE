#ifndef IO_UTIL_H
#define IO_UTIL_H
#include "global.h"

const int bufferSize= 4*1024*1024;
const int sizeInt = sizeof(int);
const int intsPerBlock = bufferSize/sizeInt;


class ReadBuffer {

	int *buf;
	int ptr;
	FILE *infile;

public:
	bool isend;
	int  nread;
	ReadBuffer() {
		buf = (int*)malloc(bufferSize);
	};
	ReadBuffer(FILE *infile_t) {
		buf = (int*)malloc(bufferSize);
		open(infile_t);
	};
	ReadBuffer(FILE *infile_t, int offset) {
		buf = (int *)malloc(bufferSize);
		open(infile_t, offset);
	}
	~ReadBuffer() {
		free(buf);
	};

	int read(int *p) {
		if (isend) return 0;
        *p = buf[ptr];

        if((++ptr)== nread){
        		if (nread < intsPerBlock)
        			isend = true;
        		else {
        			nread=fread(buf, sizeInt, intsPerBlock, infile);
        			if (nread == 0)
        				isend = true;
        			ptr = 0;
        		}
        }

        return 1;
	};

	int read(int *p, int cnt) {

		int nread = 0;
		for (int i=0; i<cnt; ++i) {
			if (this->read(p+i) ==0)
				break;
			++nread;
		}
		return nread;
	}

	void open(FILE *infile_t) {
		infile = infile_t;
		rewind(infile);
		nread = fread(buf, sizeInt, intsPerBlock, infile); 
		ptr = 0;
		isend = false; 
	};

	void open(FILE *infile_t, int offset) {
		infile = infile_t;
		rewind(infile);
		fseek(infile, offset*sizeInt, SEEK_SET);
		nread = fread(buf, sizeInt, intsPerBlock, infile);
		ptr = 0;
		isend = false;
	};
};

class WriteBuffer {

	int *buf;
public:
	FILE *outfile;
	int ptr;

public:
	WriteBuffer() {
		buf = (int*)malloc(bufferSize); // change WriteBuffer
	};

	WriteBuffer(FILE *outfile_t) {
		buf = (int*)malloc(bufferSize);
		open(outfile_t);
	};

	WriteBuffer(FILE *outfile_t, int offset) {
		buf = (int*)malloc(bufferSize);
		open(outfile_t, offset);
	};

	~WriteBuffer() {
		flush();
		free(buf);
	};

	void write(int *src, int cnt) {
		unsigned total=cnt;
		while(cnt>0){
		        if (cnt < intsPerBlock-ptr) {
		                memcpy(buf+ptr,src+(total-cnt),cnt*sizeInt);
		                ptr += cnt;
		                cnt = 0;
		        } else {//cpAmt > vidPerBlk-ptrWbuff
		                memcpy(buf+ptr,src+(total-cnt),(intsPerBlock-ptr)*sizeInt);
		                fwrite(buf,sizeInt,intsPerBlock,outfile);
		                cnt -= intsPerBlock-ptr;
		                ptr = 0;
		        }
		}
	};

	void open(FILE *outfile_t) {
		outfile = outfile_t;
		ptr = 0;
		rewind(outfile);
	}

	void open(FILE *outfile_t, int offset) {
		outfile = outfile_t;
		ptr = 0;
		rewind(outfile);
		fseek(outfile, offset*sizeInt, SEEK_SET);
	}

	void flush() {
		if(ptr>0) {
		        fwrite(buf,sizeInt,ptr,outfile);
		        ptr = 0;
		}
	};
};

#endif
