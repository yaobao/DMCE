#ifndef INSERT_H_
#define INSERT_H_

#include "global.h"
#include "io.h"
#include "tree.h"
#include "delete.h"

void subpruV(CliNode *back, bool *hav, Tree &T) {
	if(!back) return;

	if(back->chi.size() == 0){
		pruTree(back, T);
	}
	else {
		int i=0;
		while(back && i<back->chi.size()){
			CliNode *newback = back->chi[i];
			if(hav[newback->key]){
				subpruV(newback, hav, T);
				if(back && i<back->chi.size() && back->chi[i] == newback)
					++i;
			}
			else ++i;
		}
	}
}

void pruV(Block &b, ReadBuffer &rb, WriteBuffer &wb, int s, int ncand, int *cand) {
	int u, ngt;		
	rb.read(&u, 1); 
	ngt = b.gdeg[u];

	bool *hav = new bool[MAX_V];
	memset(hav, false, sizeof(bool)*MAX_V);
	
	for(int i=0; i<ncand; ++i){
		hav[cand[i]] = true;
	}
	hav[s] = true; //+++++

	Tree *T = new Tree();
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndexV(rb);
		tp = T->IndexHead[s];
	}
	else
		tp = NULL;

	while(tp!= NULL){
		CliNode *pre = tp;
		while(pre != NULL){
			if(!hav[pre->key] && pre->key != s) break; 
			pre = pre->par;
		}
		if(pre == NULL) {
			CliNode *back = tp;
			subpruV(back, hav, *T);
		}
		tp = tp->next;
	}
	T->WriteTree2(wb, u);

	delete []hav;
	delete T;
}

//prune and add
void addCli(Block &b, ReadBuffer &rb, WriteBuffer &wb, int s, int e, int ncand, int *cand) {
	int u, ngt;
	rb.read(&u, 1);
	ngt = b.gdeg[u];
	int *ugt = b.edge+b.start[u];

	int *index = new int[MAX_V];	memset(index, -1, sizeof(int)*MAX_V); 
	int cur = 0;
	bool ok = false;	
	for (int i=0; i<ngt; ++i){									
		index[ugt[i]] = cur++;
		if(ugt[i] == 0) ok = true;
	}	
	if(!ok) index[0] = cur++;		//mark, and check delete

	bool *hav = new bool[MAX_V]; 
	memset(hav, false, sizeof(bool)*MAX_V);
	for(int i=0; i<ncand; ++i){
		hav[cand[i]] = true;
	}
	hav[s] = true;	hav[e] = true;	

	Tree *T = new Tree();
	CliNode *tp = NULL;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndexV(rb);
		map<int,CliNode*>::iterator it;
		it = T->IndexHead.find(s);
		if(it != T->IndexHead.end())
			tp = T->IndexHead[s];	//
	}

	while(tp){
		CliNode *pre = tp;
		while(pre != NULL){
			if(!hav[pre->key]) break;
			pre = pre->par;
		}
		if(pre == NULL) {
			CliNode *back = tp;
			subpruV(back, hav, *T);
		}
		tp = tp->next;
	}

	tp = NULL;
	if(empty == 1){
		map<int,CliNode*>::iterator it;
		it = T->IndexHead.find(e);
		if(it != T->IndexHead.end()){
			tp = T->IndexHead[e];
		}
	}

	while(tp){
		CliNode *pre = tp;
		while(pre != NULL){
			if(!hav[pre->key]) break; 
			pre = pre->par;
		}
		if(pre == NULL) {
			CliNode *back = tp;
			subpruV(back, hav, *T);
		}
		tp = tp->next;
	}

	int *clique = new int[ngt+3];
	int nclique;
	if(u != s){
		clique[0] = u, clique[1] = s, clique[2] = e; 
		nclique = 3; 
	}
	else {
		clique[0] = s, clique[1] = e; 
		nclique = 2;
	}

	comMC(b, *T, u, ncand, cand, clique, nclique, hav, index); 
	T->WriteTree2(wb, u);

	delete []index;
	delete []clique;
	delete []hav;
	delete T;
}

//update the info in the disk
void NodeInsert(Block *b, int s, int e, int ncand, int *cand, int sldeg, int sgdeg, int *sADJ, int eldeg, int egdeg, int *eADJ) {

	int *offset = new int[MAX_V];
	int Smax;

	int r = MAX_B/nslaves;
	if(MAX_B%nslaves>=RANK)
		++r;
	for(int i=0; i<r; ++i) {
		rewind(offsetFiles[i]);
		ReadBuffer rboffset(offsetFiles[i]);
		int v, off, no;

		while(rboffset.read(&v) > 0) {
			rboffset.read(&Smax);
			rboffset.read(&off);
			offset[v] = off;
		}
	}
	int *ltse = new int[sldeg+1]; /*have both <s and <t*/
	int nltse = set_intersection(sADJ+sgdeg, sADJ+sgdeg+sldeg, eADJ+egdeg, eADJ+egdeg+eldeg, ltse)-ltse;
	ltse[nltse++] = s;
	int *lts = new int[sldeg];    /*have <s not <t*/
	int nlts = set_difference(sADJ+sgdeg, sADJ+sgdeg+sldeg, ltse, ltse+nltse, lts)-lts;
	int *lte = new int[eldeg+1];    /*have <t not <s*/
	int nlte = set_difference(eADJ+egdeg, eADJ+egdeg+eldeg, ltse, ltse+nltse, lte)-lte;
	lte[nlte++] = e;	

	for(int i=0; i<nlts; ++i) {
		int v = lts[i];
		int vto = (order[v]%MAX_B)%nslaves;
		int blk = (order[v]%MAX_B)/nslaves;
		vto++;
		if(vto == RANK) {
			ReadBuffer rb(treeFiles[blk], offset[v]);
			WriteBuffer wb(treeFiles[blk], offset[v]);
			pruV(b[blk], rb, wb, s, ncand, cand);
		}
	}
	for(int i=0; i<nlte; ++i){
		int v = lte[i];
		int vto = (order[v]%MAX_B)%nslaves;
		int blk = (order[v]%MAX_B)/nslaves;
		vto++;
		if(vto == RANK) {
			ReadBuffer rb(treeFiles[blk], offset[v]);
			WriteBuffer wb(treeFiles[blk], offset[v]);
			pruV(b[blk], rb, wb, e, ncand, cand);
		}
	}

	for(int i=0; i<nltse; ++i){
		int v = ltse[i]; 			
		int vto = (order[v]%MAX_B)%nslaves;
		int blk = (order[v]%MAX_B)/nslaves;	   
		vto++;
		if(vto == RANK) {

			ReadBuffer rb(treeFiles[blk], offset[v]);
			WriteBuffer wb(treeFiles[blk], offset[v]);
			addCli(b[blk],rb, wb, s, e, ncand, cand);
		}
	}
	delete []offset;
	delete []ltse;
	delete []lte;
	delete []lts;
}
#endif
