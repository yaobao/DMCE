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

int *sADJ, *eADJ;
int sldeg, sgdeg, sdeg, eldeg, egdeg, edeg;
int *S, *E;
Block b[MAX_B];
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

void generate() {

	if (RANK == master) {
		vector<vector<string> > assignmentPointer(nslaves);
		char buffer[100];
		for(int i=0; i<MAX_B; ++i){
			sprintf(buffer, "road_part_%d", i);
			int ito = i%nslaves;
			assignmentPointer[ito].push_back(string(buffer));
		}

		for(int i=0; i<nslaves; ++i){
			int len = (assignmentPointer)[i].size();
			MPI_Send(&len, 1, MPI_INT, i+1, lentag, MPI_COMM_WORLD);

			for(int j=0; j<len; ++j){
				string s = assignmentPointer[i][j];
				int slen = s.length();
				const char *name = s.c_str();
				MPI_Send(&slen, 1, MPI_INT, i+1, slentag, MPI_COMM_WORLD);
				MPI_Send(name, slen, MPI_CHAR, i+1, nametag, MPI_COMM_WORLD);
			}
		}
	}
	else{
		Node node;
		node.download();                    //download the subgraph from each block
		node.compute();
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(RANK == master) gather();
}

void update(char gname[], char type[]) {

	int n, s, e;
	int TypeDelete = atoi(type);

	sADJ = ALLOC(MAX_V), eADJ = ALLOC(MAX_V);
	int *cand = ALLOC(MAX_V);
	int *edge = ALLOC(MAX_V), *edge2 = ALLOC(MAX_V); 
	S = ALLOC(MAX_V), E = ALLOC(MAX_V);


	int num = 0;
	if(RANK == master){
		FILE *g = fopen(gname, "rb");
		ReadBuffer rb(g);
		while(rb.read(&S[num]) > 0){
			rb.read(&E[num]);
			num++;
		}
		fclose(g);
	}
	else {
		int r = MAX_B/nslaves;
		if(MAX_B%nslaves>=RANK)
			++r;		
		for(int i=0; i<r; ++i){
			b[i].loadFromDisk(blkFiles[i]);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&num, 1, MPI_INT, master, MPI_COMM_WORLD);
	MPI_Bcast(S, num, MPI_INT, master, MPI_COMM_WORLD);
	MPI_Bcast(E, num, MPI_INT, master, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	for(int i=0; i<num; ++i){
		int s = S[i], e = E[i];	

		int sto = (order[s]%MAX_B)%nslaves;
		int sblk = (order[s]%MAX_B)/nslaves;

		int eto = (order[e]%MAX_B)%nslaves;
		int eblk = (order[e]%MAX_B)/nslaves;
		sto++; eto++;

		if(RANK == sto) {
			sldeg = b[sblk].ldeg[s];
			sgdeg = b[sblk].gdeg[s]; 
			sdeg = sldeg+sgdeg; 
			int *p = b[sblk].edge+b[sblk].start[s];
			memcpy(sADJ, p, sizeof(int)*(sdeg));
		}
		if(RANK == eto) {
			eldeg = b[eblk].ldeg[e];
			egdeg = b[eblk].gdeg[e];
			edeg = eldeg+egdeg; 
			int *p = b[eblk].edge+b[eblk].start[e];
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
		edge2 = combine(edge2, egdeg, eldeg, eADJ);	// outputSet(eADJ, egdeg+eldeg);
		int ncand = set_intersection(edge, edge+sldeg+sgdeg, edge2, edge2+eldeg+egdeg, cand)-cand;
		MPI_Barrier(MPI_COMM_WORLD); 

		if(RANK == master)
			cout << s << ", " << e << endl;
		if(RANK != master) { 
			if(TypeDelete == 1)		
				NodeDelete(b, s, e, ncand, cand, sldeg, sgdeg, sADJ, eldeg, egdeg, eADJ, edge, edge2);
			else
				NodeInsert(b, s, e, ncand, cand, sldeg, sgdeg, sADJ, eldeg, egdeg, eADJ);
		
			int r = MAX_B/nslaves;
			if(MAX_B%nslaves>=RANK)
				++r;
			for(int j=0; j<r; ++j){
				if(TypeDelete == 1)
					b[j].deleteDisk(s,e);
				else
					b[j].insertDisk(s, e, sADJ, sgdeg, sldeg, eADJ, egdeg, eldeg);
				}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

		MPI_Barrier(MPI_COMM_WORLD);
		free(sADJ); free(eADJ);
		free(cand);
		free(edge); free(edge2);
		free(S); free(E);
	}

#endif
