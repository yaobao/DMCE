#ifndef QUERYSET_H_
#define QUERYSET_H_

#include "global.h"
#include "io.h"
#include "tree.h"
#include <algorithm>

bool exist(int u, int nset, int *set) {             /*binary serch*/

	int st = 0, ed = nset-1;
	while(st <= ed) {

		int mid = (ed+st)/2;
		if(set[mid] == u) return true;
		else if(set[mid] < u)	st = mid+1;
		else	ed = mid-1;
	}
	return false;
}

/*1. subset of V, root is the V itself*/
void que_SubsetV(ReadBuffer &rb, int *set, int nset, bool *hav) {

	int u;
	rb.read(&u, 1);

	Tree *T = new Tree();
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndex(rb);
		tp = T->LeafHead; /*to be the subset, leafnode must be the vertex in V*/
	}
	else
		tp = NULL;

	int *clique = ALLOC(MAX_V);
	int nclique = 0;

	bool ok = true;
	while(tp!= NULL){
		ok = true;
		if(hav[tp->key])
			clique[tp->lev] = tp->key;		/*useless*/

		CliNode *pre = tp;
		while(pre != NULL){
			if(hav[tp->key])
				clique[pre->lev] = pre->key;
			else{
				ok = false;
				break;
			}
			pre = pre->par;
		}
		if(ok) {
			nclique = tp->lev+1;
						//cout << "subSet_res: ";outputSet(clique, nclique);
		}
		tp = tp->next;
	}

	free(clique);
	delete T;
}

void Node_Subset(int *set, int nset) {
//	cout << nset << endl;cout << "SET: "; outputSet(set, nset);
	if(nset == 0) return;
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
	bool *hav = (bool *)malloc(sizeof(bool)*MAX_V); memset(hav, false, sizeof(bool)*MAX_V);
	for(int i=0; i<nset; ++i){
		hav[set[i]] = true;
	}
	for(int i=0; i<nset; ++i) {
		int v = set[i];
		int tp = order[v]%nblk;
		int vto = tp/r; vto++;
		int blk = tp%r;
		if(vto == RANK) {
			//if(offset[v] == -1) continue;
			ReadBuffer rb(treeFiles[blk], offset[v]);
			que_SubsetV(rb, set, nset, hav);
		}
	}

	free(hav);
	free(offset);
}


/*2. superset of V*/
/* boundest: roots \in {\cap adj(<v_i)} \cup {v_0};
   relaxest one: check adj(<v_0) and adj{v_0}, check their newcandset, it can be implemented later*/
//output all cliques whose size if larger than nsetV and check wether they are supsersets

/*improvements: 1, index[qv], or, index[set[nset-1]], but the final size is unknown; 2, index[leaf], check the size of leaf */
void que_SupsetV(ReadBuffer &rb, int *set, int nset) {
	int u;
	rb.read(&u, 1);

	Tree *T = new Tree();
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndex(rb);
		tp = T->LeafHead; 
	}
	else
		tp = NULL;

	int *clique = ALLOC(MAX_V);

	int nclique = 0;
	int *tmp = ALLOC(MAX_V);

	bool ok = true;
	while(tp!= NULL){
		ok = true;
		//cout << "tp->key: " << tp->key << ", lve: " << tp->lev << ", nset: " << nset << endl;
		if(tp->lev+1 < nset){
			tp = tp->next;
			continue;
		}
			CliNode *pre = tp;
		while(pre != NULL){
			clique[pre->lev] = pre->key;
			pre = pre->par;
		}
		nclique = tp->lev+1;
		outputSet(clique, nclique);
		sort(clique, clique+nclique);
		int ntmp = set_difference(clique, clique+nclique, set, set+nset, tmp)-tmp;
		if(ntmp == nclique-nset) {
			cout << "Supset_res: ";outputSet(clique, nclique);
		}
		tp = tp->next;
	}
	free(clique);
	delete T;
}

void Node_Supset(int *root, int nroot, int *set, int nset) {

	int *offset = ALLOC(MAX_V);
	memset(offset, -1, sizeof(int)*MAX_V);
	//outputSet(root, nroot);
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
			//if(offset[v] == -1) continue;
			ReadBuffer rb(treeFiles[blk], offset[v]);
			//cout << "start supset: " << v << endl;
			que_SupsetV(rb, set, nset);
		}
	}

	free(offset);
}
#endif
