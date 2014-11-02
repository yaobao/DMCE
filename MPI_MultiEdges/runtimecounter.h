#ifndef RUNTIMECOUNTER_H_
#define RUNTIMECOUNTER_H_
#include "global.h"
#include <sys/time.h>

class Runtimecounter{
public: //timezone tz;
	timeval t1;
	timeval t2;
	public:
		Runtimecounter();
		void start();
		void stop();
		float GetRuntime();
		float GetRuntimeusr();

};

Runtimecounter::Runtimecounter(){
}

void Runtimecounter::start(){
	gettimeofday(&this->t1, NULL);
}

void Runtimecounter::stop(){
	gettimeofday(&this->t2, NULL);
}

float Runtimecounter::GetRuntime(){
	float t=(float)(t2.tv_sec-t1.tv_sec)*1.0+(float)(t2.tv_usec-t1.tv_usec)/1000000.0;
	return t;
}
#endif /* RUNTIMECOUNTER_H_ */
