#ifndef QUERYV_H_
#define QUERYV_H_

#include "global.h"
#include "io.h"
#include "tree.h"

void subqueryV(int *clique, int nclique, CliNode *back) {

	if(back->nchi == 0){
		//cout << "q_res: "; outputSet(clique, nclique);
	}
	else {
		for(int i=0; i<back->nchi; ++i){
			CliNode *newback = back->chi[i];
			clique[nclique] = newback->key;
			subqueryV(clique, nclique+1, newback);
		}
	}
}

void queV(ReadBuffer &rb, int qv) {

	int u;
	rb.read(&u, 1);

	Tree *T = new Tree();		
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndexV(rb);
		tp = T->IndexHead[qv];
	}
	else
		tp = NULL;

	int *clique = ALLOC(MAX_V);
	int nclique=0;

	while(tp!= NULL){
		//bidirect searching the clique
		CliNode *pre = tp;
		while(pre != NULL){
			clique[pre->lev] = pre->key;
			pre = pre->par;
		}
		nclique = tp->lev+1;

		CliNode *back = tp;
		subqueryV(clique, nclique, back);			/*the problem happens in subqueryV*/
		nclique = 0;
		tp = tp->next;
	}

	free(clique);
	delete T;
}

void NodeQuery(int qv, int nroot, int *root) {

	int *offset = ALLOC(MAX_V);
	memset(offset, -1, sizeof(int)*MAX_V);

	int Smax;
	for(int i=0; i<r; ++i) {				
		ReadBuffer rboffset(offsetFiles[i]);
		int v, off;

		while(rboffset.read(&v) > 0) {
			rboffset.read(&Smax);
			rboffset.read(&off);
			offset[v] = off;
		}
	}
	for(int i=0; i<nroot; ++i) {
		int v = root[i];
		int tp = order[v]%nblk;
		int vto = tp/r; vto++;
		int blk = tp%r;
		if(vto == RANK) {
			ReadBuffer rb(treeFiles[blk], offset[v]);
			queV(rb, qv);
		}
	}

	free(offset);
}

#endif
