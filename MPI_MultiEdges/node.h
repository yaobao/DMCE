#ifndef NODE_H_
#define NODE_H_

#include "clique.h"
#include "global.h"

FILE *treeFiles[MAX_B];
FILE *offsetFiles[MAX_B];
FILE *blkFiles[MAX_B];

typedef struct Node{
	char blkname[MAX_B][200];
	int len;

	Node();
	void download();
	void compute();
} Node;

Node::Node() {
}

void Node::download() {

	MPI_Recv(&len, 1, MPI_INT, master, lentag, MPI_COMM_WORLD, &status[RANK]);
	int slen; char name[200];
	for(int i=0; i<len; ++i){
		MPI_Recv(&slen, 1, MPI_INT, master, slentag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(name, slen, MPI_CHAR, master, nametag, MPI_COMM_WORLD, &status[RANK]);
		name[slen++] = '\0';
		memcpy(blkname[i], name, slen);
	}
}

void Node::compute() {
	LL ret =0;

	for (int i=0; i<len; ++i) {	
		VI V;
		Block b;	
		blkFiles[i] = fopen(blkname[i], "rb");
		b.loadFromDisk(blkFiles[i], V);		
		WriteBuffer wbtree;
		WriteBuffer wboffset; 

		treeFiles[i] = tmpfile();
		offsetFiles[i] = tmpfile();
		wbtree.open(treeFiles[i]);
		wboffset.open(offsetFiles[i]);
		
		ret += listCliques(b, V, wbtree, wboffset);	

		wbtree.flush();
		wboffset.flush();
	}

	MPI_Send(&ret, 1, MPI_INT, master, rettag, MPI_COMM_WORLD);
}

#endif /* NODE_H_ */
