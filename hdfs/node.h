#ifndef NODE_H_
#define NODE_H_

#include "clique.h"
#include "global.h"

FILE *treeFiles[MAX_B];
FILE *offsetFiles[MAX_B];
FILE *blkFiles[MAX_B];

void load_graph(const char* inpath， char *outname){
					 FILE *fp = fopen(outname,"w");
                     hdfsFS fs = getHdfsFS();
                     hdfsFile in=getRHandle(inpath, fs);
                     LineReader reader(fs, in);
                     while(true)
                     {
                            reader.readLine();
                            if(!reader.eof()){
								fputs(reader.getLine(), fp);
							} 
                            else break;
                     }
                     hdfsCloseFile(fs, in);
                     hdfsDisconnect(fs);
                     fclose(fp);
              }

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
	char buffer[50];
	for(int i=0; i<len; ++i){
		MPI_Recv(&slen, 1, MPI_INT, master, slentag, MPI_COMM_WORLD, &status[RANK]);
		MPI_Recv(name, slen, MPI_CHAR, master, nametag, MPI_COMM_WORLD, &status[RANK]);
		name[slen++] = '\0';
		sprintf(buffer, "part_%d", i);
		memcpy(blkname[i], buffer, slen);
		load_graph(name, blkname[i]);
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
