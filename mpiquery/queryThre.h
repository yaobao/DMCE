#ifndef QUERYTHRE_H_
#define QUERYTHRE_H_

#include "global.h"
#include "io.h"
#include "tree.h"
#include "query.h"

int QueThre(ReadBuffer &rb, int thre) {
	int u;	
	int *clique = ALLOC(MAX_V);
	/*one method is to record the next u's offset, it is easy to implement*/
	rb.read(&u);

	Tree *T = new Tree();
	CliNode *tp = NULL;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeThre(rb, thre);
		tp = T->LeafHead;
	}
	else
		tp = NULL;

	int ret = 0;
	int nclique = 0;
	while(tp){
		ret++;
		CliNode *pre = tp;
		while(pre) {
			clique[pre->lev] = pre->key;
			pre = pre->par;
		}
		nclique = tp->lev+1; /*start from 0*/
		CliNode *back = tp;
		subqueryV(clique, nclique, back);
		nclique = 0;
		tp = tp->next;
	}
	delete T;

	free(clique);
	return ret;
}

void NodeThre(int thre) {

	int Smax;
	int result = 0;
	for(int i=0; i<r; ++i) {				
		ReadBuffer rboffset(offsetFiles[i]);
		int v, off;
		while(rboffset.read(&v) > 0) {
			rboffset.read(&Smax);
			rboffset.read(&off);	
			if(Smax >= thre) {
				int tp = order[v]%nblk;
				int blk = tp%r;
				ReadBuffer rb(treeFiles[blk], off);
				result += QueThre(rb, thre);
			}
		}
	}
	//printf("Thre: %d, %d\n", thre, result);
}
#endif 
