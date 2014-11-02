#ifndef BLOCK_H_
#define BLOCK_H_

#include "global.h"
#include "io.h"

struct Block{
	int n;
	int edgenum;
	int *edge;
	int *gdeg;
	int *ldeg;
	int *start;
	int *V;
	int *Vtag;
	vector<int> startV;

	Block();
	~Block();
	void clear();
	void loadFromDisk(FILE *infile);
	void loadFromDisk(FILE *infile, VI &VV);
	void insertDisk(int s, int e, int *sadj, int sgdeg, int sldeg, int *eadj, int egdeg, int eldeg);
	void deleteDisk(int s, int e);
};

Block::Block() {
	edge = ALLOC(MAX_E);
	gdeg = ALLOC(MAX_V);
	ldeg = ALLOC(MAX_V);
	start = ALLOC(MAX_V);
	V = ALLOC(MAX_V);
	Vtag = ALLOC(MAX_V);
	memset(V, 0, sizeof(int)*MAX_V);
	memset(Vtag, 0, sizeof(int)*MAX_V);
	memset(ldeg, 0, sizeof(int)*MAX_V);
	memset(gdeg, 0, sizeof(int)*MAX_V);
	memset(edge, -1, sizeof(int)*MAX_E);
	edgenum = 0;
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
	startV.resize(0);
	edgenum = 0;
}
void Block::loadFromDisk(FILE *infile) {

	clear();
	ReadBuffer rb(infile);
	if(rb.nread == 0) return;

	int u, ngt, nlt, p=0;
	int *tp = ALLOC(MAX_V); int tag;
	while (rb.read(&u) >0) {	
		if(!V[u]){
			rb.read(&tag); 
			rb.read(&gdeg[u]);
			rb.read(&ldeg[u]);
			rb.read(edge+p, gdeg[u]+ldeg[u]); 
			start[u] = p;
			p+= gdeg[u]+ldeg[u];
			V[u] = 1;
			startV.push_back(u);
			if(tag == 1)
				Vtag[u] = 1;	
		}
		else
		{
			rb.read(&tag, 1);
			rb.read(&ngt);
			rb.read(&nlt);
			rb.read(tp, ngt+nlt);
		}			
	}
	edgenum = p;
	free(tp);
}

void Block::loadFromDisk(FILE *infile, VI &VV) {

	ReadBuffer rb(infile);
	if(rb.nread == 0) return;

	int u, ngt, nlt, p=0;
	int tag;
	int *tp = ALLOC(MAX_E);
	while (rb.read(&u) >0) {        
		if(!V[u]){
			rb.read(&tag, 1);
			rb.read(&gdeg[u]);
			rb.read(&ldeg[u]);
			rb.read(edge+p, gdeg[u]+ldeg[u]);
			start[u] = p;
			p+= gdeg[u]+ldeg[u];
			V[u] = 1;
			if(tag==1)
				VV.push_back(u);
		}
		else
		{
			rb.read(&tag, 1);
			rb.read(&ngt); 
			rb.read(&nlt);
			rb.read(tp, ngt+nlt);
		}
	}
	free(tp);
}
void Block::insertDisk(int s, int e, int *sadj, int sgdeg, int sldeg, int *eadj, int egdeg, int eldeg){
	if(Vtag[s] == 1 && V[e] != 1){
		int edeg = egdeg+eldeg;
		memcpy(edge+edgenum, eadj, sizeof(int)*edeg);
		V[e] = 1;
		gdeg[e] = egdeg;
		ldeg[e] = eldeg;
		start[e] = edgenum;
		edgenum += edeg;
		startV.push_back(e);
	}
	if(V[s] == 1){
		int *p = edge+start[s];
		if(start[s] == edgenum){
			p[0] = e;
			gdeg[s]++;
			edgenum++;
		}
		else{
			int mid = binarysearch(p, gdeg[s], e);
			if(p[mid] != e){
				if(p[mid] < e)	
					mid++;
				for(int i=edgenum; i>mid; --i)
					p[i] = p[i-1];
				int j=0;
				while(j<startV.size() && startV[j] != s)
					++j;
				++j;
				while(j<startV.size()){
					start[startV[j]]++;
					++j;
				}		
				p[mid] = e;
				gdeg[s]++;
				edgenum++;	
			}
		}
	}

	if(V[e] == 1){
		int *p = edge+start[e]+gdeg[e];
		if(start[e]+gdeg[e] == edgenum){
			p[0] = s;
			ldeg[e]++;
			edgenum++;			
		}    
		else{
			int mid = binarysearch(p, ldeg[e], s);
			if(p[mid] != s){
				if(p[mid] < s)  
					mid++;
				for(int i=edgenum; i>mid; --i)
					p[i] = p[i-1];
				int j=0;
				while(j<startV.size() && startV[j] != e )
					++j;
				++j;
				while(j<startV.size()){
					start[startV[j]]++;
					++j;
				}
				p[mid] = s;
				ldeg[e]++;
				edgenum++;
			}
		}
	}
}


void Block::deleteDisk(int s, int e){
	if(V[s] == 1){
		int *p = edge+start[s];
		int mid = binarysearch(p, gdeg[s], e);
		for(int i=mid; i<gdeg[s]+ldeg[s]; ++i)
			p[i] = p[i+1];
		gdeg[s]--;
	}
	if(V[e] == 1){
		int *p = edge+start[e]+gdeg[e];
		int mid = binarysearch(p, ldeg[e], s);
		for(int i=mid; i<ldeg[e]; ++i)
			p[i] = p[i+1];
		ldeg[e]--;
	}
}
#endif /* BLOCK_H_ */
