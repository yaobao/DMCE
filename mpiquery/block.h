#ifndef BLOCK_H_
#define BLOCK_H_

#include "global.h"
#include "io.h"

struct Block{
	int *edge;
	int *gdeg;
	int *ldeg;
	int *start;
	int *V;

	Block();
	~Block();
	void clear();
	void loadFromDisk(FILE *infile);
};

Block::Block() {
	edge = ALLOC(MAX_E);
	gdeg = ALLOC(MAX_V);
	ldeg = ALLOC(MAX_V);
	start = ALLOC(MAX_V);
	V = ALLOC(MAX_V);
	memset(V, 0, sizeof(int)*MAX_V);
	memset(ldeg, 0, sizeof(int)*MAX_V);
	memset(gdeg, 0, sizeof(int)*MAX_V);
	memset(edge, -1, sizeof(int)*MAX_E);
	memset(start, -1, sizeof(int)*MAX_V);
}

//VV[i] should be kept
Block::~Block() {

	free(edge);
	free(gdeg);
	free(ldeg);
	free(start);
	free(V);
}

void Block::clear(){
	memset(V, 0, sizeof(int)*MAX_V);
	memset(ldeg, 0, sizeof(int)*MAX_V);
	memset(gdeg, 0, sizeof(int)*MAX_V);
	memset(start, -1, sizeof(int)*MAX_V);
	memset(edge, -1, sizeof(int)*MAX_E);
}
void Block::loadFromDisk(FILE *infile) {

	clear();
	ReadBuffer rb(infile);
	if(rb.nread == 0) return;

	int u, ngt, nlt, p=0;
	int *tp = ALLOC(MAX_V);
	while (rb.read(&u) >0) {	
		if(!V[u]){
			rb.read(&gdeg[u]);
			rb.read(&ldeg[u]);
			rb.read(edge+p, gdeg[u]+ldeg[u]); 
			start[u] = p;
			p+= gdeg[u]+ldeg[u];
			V[u] = 1;
		}
		else
		{
			rb.read(&ngt);
			rb.read(&nlt);
			rb.read(tp, ngt+nlt);
		}			
	}
	free(tp);
}
#endif /* BLOCK_H_ */
