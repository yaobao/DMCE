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
#include "query.h"
#include "queryThre.h"
#include "querySet.h"
#include "map"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

int *root;
int nroot;
BS *WW;
int *sADJ, *eADJ;
int sldeg, sgdeg, sdeg, eldeg, egdeg, edeg;
int MAX_DEG;
int block_size[MAX_B];
FILE *block_file[MAX_B];
char buffer[1000];
// @master: distribute graph data
void partition(char gname[]) {
	FILE *graph = fopen(gname, "rb");
	int n;
	ReadBuffer rb(graph);
	n = nnum;
	int *edge = ALLOC(MAX_E);
	int *gdeg = ALLOC(n);
	int *ldeg = ALLOC(n);
	int *start = ALLOC(n);
	int *V = ALLOC(n);
	WW = new BS[r*nslaves+1];
	int p=0;
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
	WriteBuffer *out = new WriteBuffer[nblk];
	for(int i=0; i<nblk; ++i){
		out[i].open(block_file[i]);
	}
	memset(block_size, 0, sizeof(int)*nblk);

	for(int i=0; i<n; ++i){
		int u = V[i];
		int uto = order[u]%nblk;    
		if(WW[uto][u]!=1){
			WW[uto][u]=1;
			out[uto].write(&u, 1);
			out[uto].write(&gdeg[u], 1);
			out[uto].write(&ldeg[u], 1);
			out[uto].write(edge+start[u], gdeg[u]+ldeg[u]);
			block_size[uto] += (3+gdeg[u]+ldeg[u]);
		}
		for(int j=start[u]+gdeg[u]; j<start[u]+gdeg[u]+ldeg[u]; ++j){
			int kto = order[edge[j]]%nblk;
			if(WW[kto][u]!=1){
				out[kto].write(&u, 1);
				out[kto].write(&gdeg[u], 1);
				out[kto].write(&ldeg[u], 1);
				out[kto].write(edge+start[u], gdeg[u]+ldeg[u]);
				block_size[kto] += (3+gdeg[u]+ldeg[u]);
				WW[kto][u]=1;
			}
		}
	}
	for(int i=0; i<nblk; ++i){
		out[i].flush();
	}
	delete []out;
	fclose(graph);
	delete []WW;
	free(edge); 
	free(start);
	free(gdeg);
	free(ldeg);
	free(V);
}
void distribute_partition(){
	int *tp_edge = ALLOC(MAX_E);
	ReadBuffer *in = new ReadBuffer[nblk];
	for(int i=0; i<nblk; ++i){
		in[i].open(block_file[i]);
		//		in[i].read(tp_edge[i], block_size[i]);
	}
	int end = 0;
	for(int i=0; i<nblk; ++i){
		in[i].read(tp_edge, block_size[i]);
		int uto=i%nslaves;
		int rblk=i/nslaves;
		uto++;
		MPI_Send(&end, 1, MPI_INT, uto, endtag, MPI_COMM_WORLD);
		MPI_Send(&rblk, 1, MPI_INT, uto, nlttag, MPI_COMM_WORLD);
		MPI_Send(&block_size[i], 1, MPI_INT, uto, ngttag, MPI_COMM_WORLD);
		MPI_Send(tp_edge, block_size[i], MPI_INT, uto, adjtag, MPI_COMM_WORLD);
	}
	end = 1;
	for(int i=1; i<=nslaves; ++i)
		MPI_Send(&end, 1, MPI_INT, i, endtag, MPI_COMM_WORLD);


	delete []in;
	free(tp_edge);
}
void distribute(FILE *graph) {
	int n;
	ReadBuffer rb(graph);
	//	rb.read(&n);		
	n = nnum;
	int *adj = ALLOC(n);

	int end = 0;
	MAX_DEG = 0; int numedge = 0;
	WW = new BS[r*nslaves+1];
	for (int i=0; i<n; ++i) {

		int u, ngt, nlt;
		rb.read(&u);
		rb.read(&ngt);
		rb.read(&nlt);
		rb.read(adj, ngt+nlt);	numedge += (ngt+nlt);
		int tp = order[u]%nblk;    //order[u]:0, 1, 2...
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

float generate(char gname[]) {
	float tp=0;
	if(RANK==master){
		for(int i=0; i<nblk; ++i){
			sprintf(buffer, "part_%d", i);
			block_file[i]=fopen(buffer,"wb"); 
		}
		partition(gname);
		for(int i=0; i<nblk; ++i)
			fclose(block_file[i]); 
	}

	MPI_Bcast(block_size, nblk, MPI_INT, master, MPI_COMM_WORLD); 
	MPI_Barrier(MPI_COMM_WORLD);

	if (RANK == master) {

		for(int i=0; i<nblk; ++i){
			sprintf(buffer, "part_%d", i);
			block_file[i]=fopen(buffer,"rb"); 
		}
		//partition(gname);
		distribute_partition();
		for(int i=0; i<nblk; ++i)
			fclose(block_file[i]); 
		//distribute(g);
	}
	else{

		Node node;
		node.partitiondownload();
		//		node.download();                    //download the subgraph from each block
		node.compute();
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&tp, 1, MPI_INT, master, MPI_COMM_WORLD); 
	MPI_Barrier(MPI_COMM_WORLD);

	if(RANK == master) gather();
	return tp;
}

void update(char argv3[], char argv4[], char type[]) {

	int s = atoi(argv3);
	int e = atoi(argv4);
	int TypeDelete = atoi(type);

	sADJ = ALLOC(MAX_V), eADJ = ALLOC(MAX_V);
	int *cand = ALLOC(MAX_V);
	int *edge = ALLOC(MAX_V), *edge2 = ALLOC(MAX_V); 

	Block b_s, b_e;

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

void queryV(char gname[], char vsize[]) {
	int *Qv = ALLOC(MAX_V);
	int nqv = atoi(vsize);

	if(RANK == master){

		FILE *g = fopen(gname, "rb");
		ReadBuffer rb(g);
		int n; rb.read(&n); //cout << n << endl;
		int qv;
		for(int i=0; i<nqv; ++i){
			rb.read(&qv); 
			Qv[i] = qv;	
		}
		fclose(g);
	}

	MPI_Bcast(Qv, nqv, MPI_INT, master, MPI_COMM_WORLD);
	root = ALLOC(MAX_V);

	for(int i=0; i<nqv; ++i){

		int qv = Qv[i];
		int tp = order[qv]%nblk;    //order[u]:0, 1, 2...
		int uto = tp/r; uto++;
		int blk = tp%r;

		if(RANK == uto) {
			Block b;
			b.loadFromDisk(blkFiles[blk]);
			nroot = b.ldeg[qv];
			memcpy(root, b.edge+b.start[qv]+b.gdeg[qv], nroot*sizeof(int));
			root[nroot++] = qv;	
		}

		MPI_Bcast(&nroot, 1, MPI_INT, uto, MPI_COMM_WORLD);
		MPI_Bcast(root, nroot, MPI_INT, uto, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);

		if(RANK != master) {
			NodeQuery(qv, nroot, root);				
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}		
	MPI_Barrier(MPI_COMM_WORLD);
	free(Qv);
	free(root);
}

void queryThre(char gname[], char nqthre[]) {				

	int *Qthre = ALLOC(MAX_V);
	int nthre = atoi(nqthre);

	if(RANK == master){
		FILE *g = fopen(gname, "rb");
		ReadBuffer rb(g);
		int n, thre;
		rb.read(&n);
		for(int i=0; i<nthre; ++i){
			rb.read(&thre);
			Qthre[i] = thre;
		}
		fclose(g);
	}
	//if(RANK == master) outputSet(Qthre, nthre);
	MPI_Bcast(Qthre, nthre, MPI_INT, master, MPI_COMM_WORLD);

	for(int i=0; i<nthre; ++i) {
		thre = Qthre[i]; //thre > 1
		if(RANK!=master)
			NodeThre(thre);	

		MPI_Barrier(MPI_COMM_WORLD);
	}		
	MPI_Barrier(MPI_COMM_WORLD);
	free(Qthre);
}

void querysubSet(char gname[], char test[]){

	int ntest = atoi(test);
	int *nset = ALLOC(ntest);
	int **set = (int **)malloc(sizeof(int *)*ntest);
	for(int i=0; i<ntest; ++i)
		set[i] = ALLOC(MAX_V);

	if(RANK == master){
		FILE *g = fopen(gname, "rb");
		ReadBuffer rb(g);
		int n;  
		rb.read(&n);
		for(int i=0; i<ntest; ++i) {
			rb.read(&nset[i]);
			for(int j=0; j<nset[i]; ++j)
				rb.read(&set[i][j]);
			//cout << "test: " << i << ", ";	outputSet(set[i], nset[i]);
		}
		fclose(g);
	}

	MPI_Bcast(nset, ntest, MPI_INT, master, MPI_COMM_WORLD);
	for(int i=0; i<ntest; ++i)
		MPI_Bcast(set[i], nset[i], MPI_INT, master, MPI_COMM_WORLD);

	for(int i=0; i<ntest; ++i){
		if(RANK != master)
			Node_Subset(set[i], nset[i]);
		MPI_Barrier(MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	free(nset);
	for(int i=0; i<ntest; ++i)
		free(set[i]);
	free(set);
#ifdef DEBUG
	cout << RANK << " end" << endl;
#endif
}

void querysupSet(char gname[], char test[], char ori[]) {
	if(RANK==master+1){
		Block b;
		b.loadFromDisk(blkFiles[0]);
	}

	FILE *graph = fopen(ori, "rb");    
	Block b;
	b.loadFromDisk(graph);

	root = ALLOC(MAX_V);
	int ntest = atoi(test);
	int *nset = ALLOC(ntest);
	int **set = (int **)malloc(ntest * sizeof(int *));

	for(int i=0; i<ntest; ++i)
		set[i] = ALLOC(MAX_V);

	if(RANK == master){
		FILE *g = fopen(gname, "rb");
		ReadBuffer rb(g);
		int n;
		rb.read(&n);

		for(int i=0; i<ntest; ++i) {
			rb.read(&nset[i]);
			for(int j=0; j<nset[i]; ++j)
				rb.read(&set[i][j]);
			//cout << "test " << i << ":";	outputSet(set[i], nset[i]);
		}
		fclose(g);
	}

	MPI_Bcast(nset, ntest, MPI_INT, master, MPI_COMM_WORLD);
	for(int i=0; i<ntest; ++i)
		MPI_Bcast(set[i], nset[i], MPI_INT, master, MPI_COMM_WORLD);

	for(int i=0; i<ntest; ++i){
		int v = set[i][0];
		int tp = order[v]%nblk;
		int uto = tp/r; uto++;
		if(RANK == master) {
			nroot = b.ldeg[v];
			memcpy(root, b.edge+b.start[v]+b.gdeg[v], nroot*sizeof(int));
			root[nroot++] = v;
		}
		MPI_Bcast(&nroot, 1, MPI_INT, master, MPI_COMM_WORLD);
		MPI_Bcast(root, nroot, MPI_INT, master, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
		if(RANK != master)
			Node_Supset(root, nroot, set[i], nset[i]);
		MPI_Barrier(MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	fclose(graph);
	free(root);
	free(nset);
	for(int i=0; i<ntest; ++i)
		free(set[i]);
	free(set);
}
#endif
