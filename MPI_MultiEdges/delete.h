#ifndef DELETE_H_
#define DELETE_H_

#include "global.h"
#include "io.h"
#include "tree.h"

//new clique, as 0,4,2should be removed;
void subaddPar(Tree &T, int *clique, int nclique, int *parclique, int nparclique,
		int ncand, int *cand, int nprev, int *prev, ADJ &gt, ADJ &unt, int *index, int *tmp, int dv, bool *hav) {

	if(!ncand && !nprev){														/*nparclique !=0, but it may be the empty tree originally*/
		
		T.ConstructTree(clique, parclique, nclique, nparclique);
		memcpy(parclique, clique, sizeof(int)*nclique);
		nparclique = nclique;
		return;
	}
	if (!ncand && nprev) return;
	int *V = new int[gt.cnte+1];
	int *start = new int[dv];
	int *tdeg = new int[dv+1];
	
	memcpy(V, gt.V, sizeof(int)*(gt.cnte+1));
	memcpy(start, gt.start, sizeof(int)*(dv));
	memcpy(tdeg, gt.deg, sizeof(int)*(dv+1));
	
	int *newcand = new int[ncand];
	int *newprev = new int[nprev+ncand];
	int *U = new int[ncand];
	int res = gt.max;
	int nu = set_difference(cand, cand+ncand, gt.getStart(res), gt.getStart(res)+gt.getDeg(res), U)-U;
	int ret = 0;
	for(int i=0; i<nu; ++i){
		int u = U[i], m = index[u];
		if(hav[u] == false) {
			nprev = set_union(prev, prev+nprev, U+i, U+i+1, tmp)-tmp;
			memcpy(prev, tmp, sizeof(int)*nprev);
			continue;
		}

		ncand = set_difference(cand, cand+ncand, U+i, U+i+1, tmp)-tmp;
		memcpy(cand, tmp, sizeof(int)*ncand);
		int nnewcand = set_intersection(V+start[m], V+start[m]+tdeg[m], cand, cand+ncand, newcand)-newcand;
		int nnewprev = set_intersection(prev, prev+nprev, unt.getStart(u), unt.getStart(u)+unt.getDeg(u), newprev)-newprev;
		clique[nclique] = u;

		gt.cnte = 0; gt.deg[index[gt.max]] = -1;
		for(int j=0; j<nnewcand; ++j){
			int w = newcand[j];
			int mw = index[w];
			int newDw = set_intersection(newcand, newcand+nnewcand, V+start[mw], V+start[mw]+tdeg[mw], tmp) - tmp;
			gt.add(w, tmp, newDw);
		}
		subaddPar(T, clique, nclique+1, parclique, nparclique, nnewcand, newcand, nnewprev, newprev, gt, unt, index, tmp, dv, hav);
		nprev = set_union(prev, prev+nprev, U+i, U+i+1, tmp)-tmp;
		memcpy(prev, tmp, sizeof(int)*nprev);
	}

	delete []start; delete []V; delete []tdeg;
	delete []newcand; delete []newprev; delete []U;
}

void comMC(Block &b, Tree &T, int v, int sdeg, int *sADJ, int *clique, int nclique, bool *hav, int *index)
{
	int nlt = b.ldeg[v];				
	int ngt = b.gdeg[v];
	int dv = ngt+nlt;
	dv++;
	int *pv = b.edge + b.start[v];
	int *vlt = new int[nlt];
	memcpy(vlt, pv+ngt, sizeof(int)*nlt);
	int *prev = new int[dv];
	int nprev = set_intersection(pv+ngt, pv+ngt+nlt, sADJ, sADJ+sdeg, prev)-prev;
	int *cand = new int[ngt];
	int ncand = set_intersection(pv, pv+ngt, sADJ, sADJ+sdeg, cand)-cand;


	int *tmp = new int[dv];
	int *parclique = new int[ngt+4];     //
	int nparclique = 0;
	parclique[nparclique++] = v;
	int *edge = new int[MAX_V];

	ADJ *gt = new ADJ(dv, ngt*ngt, index);		//the bug is about index[0]
	ADJ *unt = new ADJ(dv, dv*ngt, index);

	for(int i=0; i<ncand; ++i){
		int u = cand[i];
		int ugt = b.gdeg[u];
		int ult = b.ldeg[u];
		edge = combine(edge, ugt, ult, b.edge+b.start[u]);

		int Du = set_intersection(cand, cand+ncand, edge, edge+ugt+ult, tmp)-tmp;
		gt->add(u, tmp, Du);
		int Du1 = set_intersection(prev, prev+nprev, edge, edge+ugt+ult, tmp) - tmp;
		int Du2 = set_intersection(cand, cand+ncand, edge, edge+ugt+ult, tmp+Du1) - (tmp+Du1);
		unt->add(u, tmp, Du1+Du2);
	}
	subaddPar(T, clique, nclique, parclique, nparclique, ncand, cand, nprev, prev, *gt, *unt, index, tmp, dv, hav);
	
	delete []cand; delete []prev; delete []vlt;
	delete []edge; delete []parclique; delete []tmp;
	delete gt;
   	delete unt;
}

void addPar(Block &b, ReadBuffer &rb, WriteBuffer &wb, int s, int *cand, int ncand, int *sADJ, int sdeg) {
	int v, ngt;
	rb.read(&v, 1);
	ngt = b.gdeg[v];
	int *vgt = b.edge+b.start[v];

	int cur =0; bool ok = false;
	int *index = new int[MAX_V];
	for(int i=0; i<ngt; ++i){
		index[vgt[i]] = cur++;
		if(vgt[i] == 0) ok = true;
	}
	//index[v] = cur++;	//+++++ 
	if(!ok) index[0] = cur++;

	Tree *T = new Tree();
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndexV(rb);
	}
	/*add back the maximal cliques constructed by cand and s*/
	int *clique = new int[ngt+2];
	int nclique;
	if(v==s) {
		clique[0] = v; nclique = 1;
	}
	else {
		clique[0] = v, clique[1] = s;
		nclique = 2; 
	}
	bool *hav = new bool[MAX_V];
	memset(hav, false, sizeof(bool)*MAX_V);
	for(int i=0; i<ncand; ++i){
		hav[cand[i]] = true;
	}
	
	comMC(b, *T, v, sdeg, sADJ, clique, nclique, hav, index);

	delete []clique; delete []index; delete []hav;
}

void removeNode(CliNode *back, Tree &T){

	int key = back->key;
	CliNode *tp = T.IndexHead[key];
	
	if(tp == back){
		map<int,CliNode*>::iterator it;
		if(T.IndexHead[back->key]->next == NULL){
			it = T.IndexHead.find(back->key);
			T.IndexHead.erase(it);
		}
		else{
			T.IndexHead[back->key] = T.IndexHead[back->key]->next; 
		}
		if(back == T.CliTree)
			T.CliTree = NULL;
		delete back;
		back = NULL;
		return;
	}

	while(tp){
		if(tp->next == back)
			break;
		tp = tp->next;
	}
	
	if(tp->next==back){
		tp->next = back->next;
	}

	if(back == T.CliTree)	//+++
		T.CliTree = NULL;
	delete back;
	back = NULL;
}

void pruTree(CliNode *back, Tree &T) {
	if(back == NULL) return;

	CliNode *par = back->par;
	if(par == NULL){
		removeNode(back, T);
		return;
	}
	int nchi = par->chi.size();
	if(nchi > 1) {
		int i=0;     
		while(i<nchi && par->chi[i]!=back){
			i++;
		}
		par->chi[i] = par->chi[nchi-1];
		par->chi.resize(nchi-1);
		removeNode(back, T);
	}
	else {
		par->chi.resize(0);
		removeNode(back, T);
		pruTree(par, T);
	}
}

//after pruned, back would be NULL;
void subpruV2(CliNode *back, bool *hav, bool ok, int e, Tree &T) {
	if(!back) return;

	if(back->chi.size() == 0 && ok){
		pruTree(back, T);
	}
	else {
		int i=0;
		while(i<back->chi.size()){
			CliNode *newback = back->chi[i];
			++i;
			int ttt = newback->key;
			if(newback->key == e) ok = true;
			if(!hav[newback->key]) continue;
			subpruV2(newback, hav, ok, e, T);
			if(back == NULL)	return;
		}
	}
}

void modify(Block &b, ReadBuffer &rb, WriteBuffer &wb, int s, int e, int ncand, int *cand, int sdeg, int *sADJ, int edeg, int *eADJ) {

	int v, ngt;
	rb.read(&v, 1);
	ngt = b.gdeg[v];
	int *vgt = b.edge+b.start[v];

	int *index = new int[MAX_V];
	int cur = 0;
	bool ok = false;
	for (int i=0; i<ngt; ++i){
		index[vgt[i]] = cur++;
		if(vgt[i] == 0) ok = true;
	}
	index[v] = cur++;
	if(!ok && v!=0) index[0] = cur++;

	Tree *T = new Tree();
	CliNode *tp;
	int empty;
	rb.read(&empty);
	if(empty == 1) {
		T->ConstructTreeIndexV(rb);
		tp = T->IndexHead[s];
	}
	else{
		tp = NULL;
	}
	/*prune contain both s, and e and cand*/;
	bool *hav = new bool[MAX_V];
	memset(hav, false, sizeof(bool)*MAX_V);
	for(int i=0; i<ncand; ++i){
		hav[cand[i]] = true;
	}
	hav[s] = true; hav[e] = true;
	ok = false;

	while(tp!= NULL){
		CliNode *pre = tp;
		while(pre != NULL){
			if(!hav[pre->key]) break;
			if(pre->key == e) ok = true;
			pre = pre->par;
		}
		if(pre == NULL) {
			CliNode *back = tp;
			subpruV2(back, hav, ok, e, *T);
		}
		tp = tp->next;
		ok = false;
	}
	int *clique = new int[ngt+2];
	int nclique;
	if(v==s) {
		clique[0] = v; nclique = 1;
	}
	else {
		clique[0] = v, clique[1] = s;
		nclique = 2;
	}

	comMC(b, *T, v, sdeg, sADJ, clique, nclique, hav, index);

	if(v!=s) {
		clique[0] = v, clique[1] = e;
		nclique = 2;
		comMC(b, *T, v, edeg, eADJ, clique, nclique, hav, index);
	}

	delete []index;
	delete []clique;
	delete []hav;
}
//1. delete all par\cup {s,e}
//2. add par\cup{s} or par\cup {e} which is maximal back;
//3. update the info in the disk about the original graph;
void NodeDelete(Block *b, int s, int e, int ncand, int *cand, int sldeg, int sgdeg, int *sADJ, int eldeg, int egdeg, int *eADJ, int *edge1, int *edge2) {

	int *offset = new int[MAX_V];
	int Smax;

	int r = MAX_B/nslaves;
	if(MAX_B%nslaves>=RANK)
		++r;

	for(int i=0; i<r; ++i) {
		ReadBuffer rboffset(offsetFiles[i]);
		int v, off, no;

		while(rboffset.read(&v) > 0) {
			rboffset.read(&Smax);
			rboffset.read(&off);
			offset[v] = off;
		}
	}

	int *ltse = new int[sldeg+1]; //have both <s and <t
	int nltse = set_intersection(sADJ+sgdeg, sADJ+sgdeg+sldeg, eADJ+egdeg, eADJ+egdeg+eldeg, ltse)-ltse;
	ltse[nltse++] = s;
	int *lts = new int[sldeg];    //have <s not <t

	int nlts = set_difference(sADJ+sgdeg, sADJ+sgdeg+sldeg, ltse, ltse+nltse, lts)-lts;
	int *lte = new int[eldeg+1];    //have <t not <s
	int nlte = set_difference(eADJ+egdeg, eADJ+egdeg+eldeg, ltse, ltse+nltse, lte)-lte;
	int *newsADJ = new int[sldeg+sgdeg], *neweADJ = new int[eldeg+egdeg];
	int sdeg = set_difference(edge1, edge1+sldeg+sgdeg, &e, &e+1, newsADJ)-newsADJ;
	int edeg = set_difference(edge2, edge2+eldeg+egdeg, &s, &s+1, neweADJ)-neweADJ;

	for(int i=0; i<nlts; ++i) {
		int v = lts[i];
		int vto = (order[v]%MAX_B)%nslaves;
		int blk = (order[v]%MAX_B)/nslaves;
		vto++;
		if(vto == RANK) {
			int nlt = b[blk].ldeg[v];
			int ngt = b[blk].gdeg[v];
			int dv = ngt+nlt;
			int *pv = b[blk].edge + b[blk].start[v];
			bool toadd = false;
			for(int j=0; j<ngt; ++j){
				if(pv[j]==e){
					toadd = true;
					break;
				}  
			}      
			if(toadd == false) continue;

			ReadBuffer rb(treeFiles[blk], offset[v]);
			WriteBuffer wb(treeFiles[blk], offset[v]);
			addPar(b[blk], rb, wb, s, cand, ncand, newsADJ, sdeg);         //before delete, {par}\cup {v, s} is maximal, but now it is
			// {par} \in {cand(v)\cap adj(s) \cap adj(e)}
		}
	}

	for(int i=0; i<nlte; ++i){
		int v = lte[i];
		int vto = (order[v]%MAX_B)%nslaves;
		int blk = (order[v]%MAX_B)/nslaves;
		vto++;
		if(vto == RANK) {
			int nlt = b[blk].ldeg[v];
			int ngt = b[blk].gdeg[v];
			int dv = ngt+nlt;
			int *pv = b[blk].edge + b[blk].start[v];
			bool toadd = false;
			for(int j=0; j<ngt; ++j){
				if(pv[j]==s){
					toadd = true;
					break;
				}
			}
			if(toadd == false) continue;

			ReadBuffer rb(treeFiles[blk], offset[v]);
			WriteBuffer wb(treeFiles[blk], offset[v]);
			addPar(b[blk], rb, wb, e, cand, ncand, neweADJ, edeg);
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
			modify(b[blk], rb, wb, s, e, ncand, cand, sdeg, newsADJ, edeg, neweADJ); //prune{s, e, cand}, add back later
		}
	}

	delete []newsADJ; delete []neweADJ; delete[]ltse;
	delete []lte; delete []lts; delete []offset;
}

#endif
