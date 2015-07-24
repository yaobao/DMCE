#ifndef CLIQUE_H_
#define CLIQUE_H_

#include "global.h"
#include "block.h"
#include "ADJ.h"
#include "io.h"
#include "tree.h"

//int thread_id;

//add parclique and Tree T
LL Clique(int *clique, int nclique, int *parclique, int &nparclique, int *cand, int ncand, int *prev, int nprev, ADJ &gt, ADJ &unt,
		int *index, int *count, int dv, int v, Tree &T, int cliv, int *tmp, int &Smax) {

	if (!ncand && !nprev) {
		count[nclique]++;
		if(Smax < nclique)	Smax = nclique;
		T.ConstructTree(clique, parclique, nclique, nparclique);
		memcpy(parclique, clique, sizeof(int)*nclique);
		nparclique = nclique;
		//		cout << "original clique: "; outputSet(clique, nclique);
		return 1;
	}
	if (!ncand && nprev) return 0;

	int *V = ALLOC(gt.cnte+1);
	int *start = ALLOC(dv);
	int *tdeg = ALLOC(dv+1);
	memcpy(V, gt.V, sizeof(int)*(gt.cnte+1));
	memcpy(start, gt.start, sizeof(int)*(dv));
	memcpy(tdeg, gt.deg, sizeof(int)*(dv+1));	//++;

	int *newcand = ALLOC(ncand), *newprev = ALLOC(nprev+ncand);
	int *U = ALLOC(ncand);
	int res = gt.max;
	int nu = set_difference(cand, cand+ncand, gt.getStart(res), gt.getStart(res)+gt.getDeg(res), U)-U;

	LL ret = 0;
	for(int i=0; i<nu; ++i){
		int u = U[i], m = index[u];
		ncand = set_difference(cand, cand+ncand, U+i, U+i+1, tmp)-tmp;
		memcpy(cand, tmp, sizeof(int)*ncand);
		int newncand = set_intersection(V+start[m], V+start[m]+tdeg[m], cand, cand+ncand, newcand)-newcand;
		int newnprev = set_intersection(prev, prev+nprev, unt.getStart(u), unt.getStart(u)+unt.getDeg(u), newprev)-newprev;
		clique[nclique] = u;

		gt.cnte = 0; gt.deg[index[gt.max]] = -1;
		for(int j=0; j<newncand; ++j){
			int w = newcand[j];
			int mw = index[w];
			int newDw = set_intersection(newcand, newcand+newncand, V+start[mw], V+start[mw]+tdeg[mw], tmp) - tmp;
			gt.add(w, tmp, newDw);
		}
		ret += Clique(clique, nclique+1, parclique, nparclique, newcand, newncand, newprev, newnprev, gt, unt, index, count, dv, v, T, u, tmp, Smax);

		nprev = set_union(prev, prev+nprev, U+i, U+i+1, tmp)-tmp;
		memcpy(prev, tmp, sizeof(int)*nprev);
	}

	free(start); free(V); free(tdeg);
	free(newcand);
	free(newprev);
	free(U);
	return ret;
}

int listCliques(Block &b, VI &V, WriteBuffer &wbtree, WriteBuffer &wboffset) {

	int *ret = ALLOC(MAX_V); memset(ret, 0, sizeof(int)*MAX_V);
	int *tmp = ALLOC(MAX_V);
	int *clique = ALLOC(MAX_V); 
	int *count = ALLOC(MAX_V);
	memset(count, 0, sizeof(int)*MAX_V);
	int *index = ALLOC(MAX_V);
	int *parclique = ALLOC(MAX_V);
	int *edge = ALLOC(MAX_V);
	int *vlt = ALLOC(MAX_V), *vgt = ALLOC(MAX_V);
	memset(index, -1, sizeof(int)*MAX_V);

	int offset = 0;
	for (int i=0; i<V.size(); ++i) {											

		int v = V[i], ngt, nlt, dv;
		clique[0] = v;
		ngt = b.gdeg[v];
		nlt = b.ldeg[v];
		dv = ngt+nlt;
		int *pv = b.edge + b.start[v];
		memcpy(vgt, pv, sizeof(int)*ngt);
		memcpy(vlt, pv+ngt, sizeof(int)*nlt);
		int cur = 0;
		bool ok = false;			
		for (int j=0; j<dv; ++j) {
			index[pv[j]] = cur++;
			if(pv[j] == 0) ok = true;
		}
		//index[v] = cur++;
		if(!ok) index[0] = cur++;

		ADJ *gt = new ADJ(dv, ngt*ngt, index);
		ADJ *unt = new ADJ(dv, dv*ngt, index);
		Tree *T = new Tree();
		for(int j=0; j<ngt; ++j){
			int u = pv[j];
			int ugt = b.gdeg[u];
			int ult = b.ldeg[u];
			edge = combine(edge, ugt, ult, b.edge+b.start[u]);

			int Du = set_intersection(pv, pv+ngt, edge, edge+ugt+ult, tmp)-tmp;
			gt->add(u, tmp, Du);
			int Du1 = set_intersection(pv+ngt, pv+ngt+nlt, edge, edge+ugt+ult, tmp) - tmp;
			int Du2 = set_intersection(pv, pv+ngt, edge, edge+ugt+ult, tmp+Du1) - (tmp+Du1);
			sort(tmp, tmp+Du1+Du2);
			unt->add(u, tmp, Du1+Du2);
		}
		int nparclique = 0;
		int Smax = 0;

		ret[v] = Clique(clique, 1, parclique, nparclique, vgt, ngt, vlt, nlt, *gt, *unt, index, count, dv, v, *T, v, tmp, Smax);
		T->WriteTree(wbtree, wboffset, offset, v, Smax);
		delete unt;
		delete gt;
		delete T;
	}

	free(index);
	free(clique);
	free(tmp);
	free(count);
	free(parclique);
	free(edge);
	free(vlt); free(vgt);

	int result = 0;
	for(int i=0; i<V.size(); ++i)
		result += ret[V[i]];
	return result;
}


#endif /* CLIQUE_H_ */
