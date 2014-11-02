#ifndef ADJ_H_
#define ADJ_H_

#include "global.h"
#include "io.h"

struct ADJ{
	int cnte;
	int *V, *deg;
	int *start;
	int *index;
	int max;

	ADJ(int nv, int ne, int *index);
	int* getStart(int u);
	int getDeg(int u);
	void add(int u, int *a, int d);
	~ADJ();
};

ADJ::ADJ(int nv, int ne, int *_index) {
	cnte = 0;
	V = new int[ne];
	memset(V, 0, sizeof(int)*ne);
	start = new int[nv];
	deg = new int[nv+1];
	index = _index;
	max = 0;		
	deg[index[max]] = -1;
}

ADJ::~ADJ() {
	delete []V;
	delete []deg;
	delete []start;
}

int* ADJ::getStart(int u)
{
	return V + start[index[u]];
}

int ADJ::getDeg(int u)
{
	return deg[index[u]];
}

void ADJ::add(int u, int *a, int d) {
	int i = index[u]; 
	start[i] = cnte;
	deg[i] = d;
	memcpy(V+cnte, a, sizeof(int)*d);
	cnte += d;
	if(d > deg[index[max]]) max = u;
}
#endif /* ADJ_H_ */
