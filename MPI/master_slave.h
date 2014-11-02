#ifndef MASTER_SLAVE_H_
#define MASTER_SLAVE_H_

#include "global.h"
#include "block.h"
#include "node.h"
#include "ADJ.h"
#include "io.h"
#include "tree.h"
#include "insert.h"
#include "delete.h"
#include "runtimecounter.h"

BS *WW;
int *sADJ, *eADJ;
int sldeg, sgdeg, sdeg, eldeg, egdeg, edeg;
int MAX_DEG;
// @master: distribute graph data
void distribute(FILE *graph) {
	int n;
	ReadBuffer rb(graph);
//	rb.read(&n);		
	n = nnum;
	int *adj = ALLOC(n);
	WW = new BS[r*nslaves+1];
	
	int end = 0;
	MAX_DEG = 0; int numedge = 0;
	for (int i=0; i<n; ++i) {

		int u, ngt, nlt;
		rb.read(&u);
		rb.read(&ngt);
		rb.read(&nlt);
		rb.read(adj, ngt+nlt);	numedge += (ngt+nlt);
		int tp = order[u]%nblk;    /*order[u]:0, 1, 2...*/
		int uto = tp/r;
		int blk = tp%r;
        uto++;

		int flag = 1;		
		WW[tp][u] = 1;
		MPI_Send(&end, 1, MPI_INT, uto, endtag, MPI_COMM_WORLD);
		MPI_Send(&u, 1, MPI_INT, uto, utag, MPI_COMM_WORLD);
		MPI_Send(&ngt, 1, MPI_INT, uto, ngttag, MPI_COMM_WORLD);
		MPI_Send(&nlt, 1, MPI_INT, uto, nlttag, MPI_COMM_WORLD);
		MPI_Send(adj, ngt+nlt, MPI_INT, uto, adjtag, MPI_COMM_WORLD);
		MPI_Send(&blk, 1, MPI_INT, uto, blktag, MPI_COMM_WORLD);
		MPI_Send(&flag, 1, MPI_INT, uto, flagtag, MPI_COMM_WORLD);

		flag = 0;
		for(int j=ngt; j<ngt+nlt; ++j){
			int tpk = order[adj[j]]%nblk;
			int kto = tpk/r;
            kto++;

			blk = tpk%r;
			if(!WW[tpk][u]){
				WW[tpk][u] = 1;
				MPI_Send(&end, 1, MPI_INT, kto, endtag, MPI_COMM_WORLD);
				MPI_Send(&u, 1, MPI_INT, kto, utag, MPI_COMM_WORLD);
				MPI_Send(&ngt, 1, MPI_INT, kto, ngttag, MPI_COMM_WORLD);
				MPI_Send(&nlt, 1, MPI_INT, kto, nlttag, MPI_COMM_WORLD);
				MPI_Send(adj, ngt+nlt, MPI_INT, kto, adjtag, MPI_COMM_WORLD);
				MPI_Send(&blk, 1, MPI_INT, kto, blktag, MPI_COMM_WORLD);
				MPI_Send(&flag, 1, MPI_INT, kto, flagtag, MPI_COMM_WORLD);
			}
		}
	}

	end = 1;
	for(int i=1; i<=nslaves; ++i)
		MPI_Send(&end, 1, MPI_INT, i, endtag, MPI_COMM_WORLD);

	free(adj);
	delete []WW;
}

// @master: collect answers
void gather() {

	int ans= 0;
	for (int i=1; i<=nslaves; ++i) {
		int pans;
		MPI_Recv(&pans, 1, MPI_INT, i, rettag, MPI_COMM_WORLD, &status[i]);
		ans+= pans;
	}
	cout << "ans: " << ans << endl;
}

void generate(char gname[]) {

	int tp;
	if (RANK == master) {
		FILE *g = fopen(gname, "rb");
		distribute(g);
		fclose(g);
	}
	else{
		Node node;
		node.download();                    //download the subgraph from each block
		node.compute();
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(RANK == master) gather();
}

void update(char argv3[], char argv4[], char type[]) {

	int s = atoi(argv3);
	int e = atoi(argv4);
	int TypeDelete = atoi(type);

	sADJ = ALLOC(MAX_V), eADJ = ALLOC(MAX_V);
	int *cand = ALLOC(MAX_V);
	int *edge = ALLOC(MAX_V), *edge2 = ALLOC(MAX_V); 

	Block b_s, b_e;

	int count = 0;

	int tp = order[s]%nblk;
	int sto = tp/r;
	int sblk = tp%r;	sto++;
	tp = order[e]%nblk;
	int eto = tp/r;
	int eblk = tp%r;	eto++;

	if(RANK == sto && RANK == eto) {
		b_s.clear();
		b_s.loadFromDisk(blkFiles[sblk]);
		sldeg = b_s.ldeg[s];
		sgdeg = b_s.gdeg[s];
		sdeg = sldeg+sgdeg; 
		int *p = b_s.edge+b_s.start[s];
		memcpy(sADJ, p, sizeof(int)*(sdeg));


		if(sblk == eblk){
			eldeg = b_s.ldeg[e];
			egdeg = b_s.gdeg[e];
			edeg = eldeg+egdeg;
			p = b_s.edge+b_s.start[e];
		}
		else{
			b_e.clear();
			b_e.loadFromDisk(blkFiles[eblk]);
			eldeg = b_e.ldeg[e];
			egdeg = b_e.gdeg[e];
			edeg = eldeg+egdeg;
			p = b_e.edge+b_e.start[e];
		}
		memcpy(eADJ, p, sizeof(int)*(edeg));
	}
	else if(RANK == sto) {
		b_s.clear();
		b_s.loadFromDisk(blkFiles[sblk]);
		sldeg = b_s.ldeg[s];
		sgdeg = b_s.gdeg[s];
		sdeg = sldeg+sgdeg;
		int *p = b_s.edge+b_s.start[s];
		memcpy(sADJ, p, sizeof(int)*(sdeg));
	}
	else if(RANK == eto) {
		b_e.clear();
		b_e.loadFromDisk(blkFiles[eblk]);
		eldeg = b_e.ldeg[e];
		egdeg = b_e.gdeg[e];
		edeg = eldeg+egdeg; 
		int *p = b_e.edge+b_e.start[e];
		memcpy(eADJ, p, sizeof(int)*(edeg));

	}

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&sldeg, 1, MPI_INT, sto, MPI_COMM_WORLD);
	MPI_Bcast(&sgdeg, 1, MPI_INT, sto, MPI_COMM_WORLD);
	MPI_Bcast(&eldeg, 1, MPI_INT, eto, MPI_COMM_WORLD);
	MPI_Bcast(&egdeg, 1, MPI_INT, eto, MPI_COMM_WORLD);
	MPI_Bcast(sADJ, sldeg+sgdeg, MPI_INT, sto, MPI_COMM_WORLD);
	MPI_Bcast(eADJ, eldeg+egdeg, MPI_INT, eto, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	edge = combine(edge, sgdeg, sldeg, sADJ);
	edge2 = combine(edge2, egdeg, eldeg, eADJ);	
	int ncand = set_intersection(edge, edge+sldeg+sgdeg, edge2, edge2+eldeg+egdeg, cand)-cand;
	MPI_Barrier(MPI_COMM_WORLD);

	if(RANK != master) {
		if(TypeDelete == 1)
			NodeDelete(s, e, ncand, cand, sldeg, sgdeg, sADJ, eldeg, egdeg, eADJ, edge, edge2);
		else
			NodeInsert(s, e, ncand, cand, sldeg, sgdeg, sADJ, eldeg, egdeg, eADJ);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	free(sADJ);
	free(eADJ);
	free(cand);
	free(edge);
	free(edge2);
}

#endif
