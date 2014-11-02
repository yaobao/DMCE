#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <queue>
#include "io.h"
#include <fstream>
#include <map>

using namespace std;

const int MAX_V = 3000000;
const int MAX_E = 100000000;
const int MAX_B = 4;

#define ALLOC(N) (int*)malloc((N)*sizeof(int))

int *order;
int *edge;
int *gdeg, *ldeg;
int *V;
int *start;

map<int, int> mark;
char buffer[50];

int main(int argc, char **argv){

	order = ALLOC(MAX_V); memset(order, 0, sizeof(int)*MAX_V);	
	FILE *gorder = fopen(argv[2], "rb");
	ReadBuffer rborder(gorder); 
	int v, vo; 
	while(!rborder.isend) {
		rborder.read(&vo); 
		rborder.read(&v);
		order[v] = vo; 
	}	
	fclose(gorder);

	int n = atoi(argv[3]);
	FILE *graph = fopen(argv[1], "rb");
	ReadBuffer rb(graph);

	edge = ALLOC(MAX_E);
	gdeg = ALLOC(MAX_V);
	ldeg = ALLOC(MAX_V);
	start = ALLOC(MAX_V);
	V = ALLOC(MAX_V);

	int p = 0;
	for (int i=0; i<n; ++i) {
		int u, ngt, nlt;
		rb.read(&u);   
		rb.read(&ngt);
		rb.read(&nlt);
		rb.read(edge+p, ngt+nlt);

		V[i] = u;
		gdeg[u] = ngt;
		ldeg[u] = nlt;
		start[u] = p; 
		p += (ngt+nlt);

	}

	FILE *wfile[MAX_B];
	for(int i=0; i<MAX_B; ++i){
		sprintf(buffer, "part_%d", i);
		wfile[i]=fopen(buffer,"w");
	}

	pair<map<int,int>::iterator,bool> ret;

	bitset<MAX_V> WW[MAX_B];
	for(int i=0; i<n; ++i){
		int u = V[i];
		int uto = order[u]%MAX_B;      
		ret = mark.insert(make_pair(u, uto));
		int tag = 1;
		WW[uto][u]=1;
		fprintf(wfile[uto], "%d %d %d %d", u, tag, gdeg[u], ldeg[u]); 
		for(int j=0; j<gdeg[u]+ldeg[u]; ++j)
			fprintf(wfile[uto], " %d", edge[start[u]+j]);
		fprintf(wfile[uto], "\n");

		tag = 0;
		for(int j=start[u]+gdeg[u]; j<start[u]+gdeg[u]+ldeg[u]; ++j){
			int kto = order[edge[j]]%MAX_B;
			ret = mark.insert(make_pair(u, kto));	
			if(WW[kto][u]!=1){
				fprintf(wfile[kto], "%d %d %d %d", u, tag, gdeg[u], ldeg[u]);
				for(int j=0; j<gdeg[u]+ldeg[u]; ++j)
					fprintf(wfile[kto], " %d", edge[start[u]+j]);
				fprintf(wfile[kto], "\n");
			}
		}
	}

	for(int i=0; i<MAX_B; ++i){
		fclose(wfile[i]);
	}

	free(edge); free(gdeg); free(ldeg); 
	free(start); free(V); free(order);
	return 0;
}
