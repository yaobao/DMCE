#ifndef NODE_H_
#define NODE_H_

#include "clique.h"
#include "global.h"

FILE *treeFiles[MAX_B];
FILE *offsetFiles[MAX_B];
FILE *blkFiles[MAX_B];

typedef struct Node{
	int nlb;
	VI VV[MAX_B];

	Node();
	void partitiondownload();
	void download();
	void compute();
	void sendbackv(int v);
} Node;

Node::Node() {
	nlb = r;
}
void Node::partitiondownload() {

	for(int i=0; i<nnum; ++i){
		int tp = order[i]%nblk;
		int uto = tp%nslaves; uto++;
		int blk=tp/nslaves;
		if(uto==RANK)
		 VV[blk].push_back(i);
	}
	WriteBuffer* wb = new WriteBuffer[nlb];
	for (int i=0; i<nlb; ++i){
		blkFiles[i] = tmpfile();					
		wb[i].open(blkFiles[i]);					
	}
	int *adj = ALLOC(MAX_E);
	int end=0, total_size=0;
	int blk = 0;
	while(MPI_Recv(&end, 1, MPI_INT, master, endtag, MPI_COMM_WORLD, &status[RANK])==MPI_SUCCESS && !end){
		MPI_Recv(&blk, 1, MPI_INT, master, nlttag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(&total_size, 1, MPI_INT, master, ngttag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(adj, total_size, MPI_INT, master, adjtag, MPI_COMM_WORLD, &status[RANK]);
		wb[blk].write(adj, total_size);
	}
	for (int i=0; i<nlb; ++i)
		wb[i].flush();
	delete []wb;
	free(adj);
}


void Node::download() {

	WriteBuffer* wb = new WriteBuffer[nlb];
	for (int i=0; i<nlb; ++i){

		blkFiles[i] = tmpfile();					
		wb[i].open(blkFiles[i]);					
	}
	int *adj = ALLOC(MAX_V);
	BS *tp = new BS[nlb];		/*bitset初始化为0*/

	int end;
	while(MPI_Recv(&end, 1, MPI_INT, master, endtag, MPI_COMM_WORLD, &status[RANK])==MPI_SUCCESS && !end){

		int u, ngt, nlt, flag, blk;
		MPI_Recv(&u, 1, MPI_INT, master, utag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(&ngt, 1, MPI_INT, master, ngttag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(&nlt, 1, MPI_INT, master, nlttag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(adj, ngt+nlt, MPI_INT, master, adjtag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(&blk, 1, MPI_INT, master, blktag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(&flag, 1, MPI_INT, master, flagtag, MPI_COMM_WORLD, &status[RANK]);

		//		printf("%d : %d %d\n", u, ngt, nlt);
		if(!tp[blk][u]){
			tp[blk][u] = 1;
			wb[blk].write(&u, 1);
			wb[blk].write(&ngt, 1);
			wb[blk].write(&nlt, 1);
			wb[blk].write(adj, ngt+nlt);
		}
		if(flag)
			VV[blk].push_back(u);

	}
	free(adj);
	for (int i=0; i<nlb; ++i)
		wb[i].flush();
	delete []wb;
	delete []tp;
}

void Node::compute() {
	LL ret =0;

	for (int i=0; i<nlb; ++i) {	
		//	for(int j=0; j<VV[i].size(); ++j)
		//	cout << VV[i][j] << " ";
		//	cout << endl;
			
		Block b;
		b.loadFromDisk(blkFiles[i]);		
		WriteBuffer wbtree;
		WriteBuffer wboffset; 


		treeFiles[i] = tmpfile();
		offsetFiles[i] = tmpfile();
		wbtree.open(treeFiles[i]);
		wboffset.open(offsetFiles[i]);
		ret += listCliques(b, VV[i], wbtree, wboffset);	

		wbtree.flush();
		wboffset.flush();

	}

	MPI_Send(&ret, 1, MPI_INT, master, rettag, MPI_COMM_WORLD);
}

void Node::sendbackv(int v) {

	int tp = order[v]%nblk;         
	int blk = tp%r;  

	Block b;
	b.loadFromDisk(blkFiles[blk]);

	int deg = b.ldeg[v];
	MPI_Send(&deg, 1, MPI_INT, master, nlttag, MPI_COMM_WORLD);
	MPI_Send(b.edge+b.start[v]+b.gdeg[v], deg, MPI_INT, master, adjtag, MPI_COMM_WORLD); /*lgt*/ 
}
#endif /* NODE_H_ */
