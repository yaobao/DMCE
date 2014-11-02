#ifndef TREE_H_
#define TREE_H_

#include "global.h"
#include "io.h"
#include <stack>

struct CliNode{
	int key, nchi, lev;
	CliNode *par, *next;
	vector<CliNode *>chi; //**chi

	CliNode();
	~CliNode();
};

CliNode:: CliNode(){

	key = -1;
	nchi = 0;
	lev = 0;
	par = NULL;
	next = NULL;
	chi.resize(0);
}

CliNode:: ~CliNode(){
	par = NULL, next = NULL;
	chi.resize(0);
}

struct Tree{
	map<int, CliNode*> IndexHead;
	CliNode *CliTree;		// 3. root
	stack<CliNode *> q;

	Tree();
	~Tree();
	void InsertNode(int v, CliNode *&parent);
	void ConstructTree(int *clique, int *parclique, int nclique, int nparclique);
	void WriteTree (WriteBuffer &wbtree, WriteBuffer &wboffset, int &offset, int v, int Smax);
	void WriteTree2 (WriteBuffer &wbtree, int v);
	void ConstructTreeIndexV(ReadBuffer &rb);
};

Tree:: Tree(){

	CliTree = NULL;			/*head*/
}

Tree:: ~Tree(){			/*delete the all tree*/

	CliNode *tp;
	while(!q.empty()) {
		tp = q.top();
		q.pop();
		if(tp)	delete tp;
		tp = NULL;
	}

	CliTree = NULL;
	IndexHead.clear();
}
void Tree:: InsertNode(int v, CliNode *&parent) {

	CliNode *newNode = new CliNode;		

	newNode->key = v;				
	newNode->par = parent; 
	newNode->lev = parent->lev+1;					/*lev start from 0*/
	parent->chi.push_back(newNode);
	parent = newNode; 
	q.push(newNode);

	map<int,CliNode*>::iterator it;
	it = IndexHead.find(v);	
	if(it == IndexHead.end()) {
		IndexHead.insert(make_pair(v, newNode));
	}
	else {
		newNode->next = IndexHead[v];
		IndexHead[v] = newNode;
	}			
}

void Tree:: ConstructTree(int *clique, int *parclique, int nclique, int nparclique) {

	if(CliTree == NULL) {
		CliNode *tp = new CliNode;
		tp->key = clique[0];
		tp->lev = 0;
		CliTree = tp;				
		CliNode *par = CliTree;	
		q.push(tp);

		IndexHead.insert(make_pair(clique[0], CliTree));

		for(int i=1; i<nclique; ++i) {
			InsertNode(clique[i], par);
			//par = IndexHead[clique[i]];       //++; par right?
		}
	}
	else {
		int *resclique = new int[nclique];
		int nresclique = 0;
		int i;
		for(i=0; i<nclique && i<nparclique; ++i){
			if(clique[i] != parclique[i])
				break;
		}
		while(i<nclique){
			resclique[nresclique++] = clique[i++];
		}

		int commv = clique[nclique-nresclique-1];
		CliNode *par = IndexHead[commv];

		for(int i=0; i<nresclique; ++i){
			InsertNode(resclique[i], par);				
		}
		delete []resclique;
	}
}

void Tree:: ConstructTreeIndexV(ReadBuffer &rb) {

	int v, ncand, lev;
	rb.read(&v);
	rb.read(&ncand);
	rb.read(&lev);		
	CliTree = new CliNode;
	CliTree->key = v;
	CliTree->nchi = ncand;
	CliTree->lev = lev;

	queue<CliNode *> q;
	q.push(CliTree);

	while(!q.empty()) {

		CliNode *tp = q.front();
		q.pop();
		int nchi = tp->nchi;

		map<int,CliNode*>::iterator it;
		it = IndexHead.find(tp->key); 
			
		if(it == IndexHead.end()){
			IndexHead.insert(make_pair(tp->key, tp));   
		}		
		else {
			tp->next = IndexHead[tp->key];
			IndexHead[tp->key] = tp;		
		}

		for(int i=0; i<nchi; ++i) {
			rb.read(&v); 
			rb.read(&ncand);	
			rb.read(&lev);	

			CliNode *tchild = new CliNode;
			tchild->key = v;
			tchild->nchi = ncand;
			tchild->lev = lev;
			tchild->par = tp;
			tp->chi.push_back(tchild);
			q.push(tchild);
		}
	}
}


//add print info
void Tree:: WriteTree (WriteBuffer &wbtree, WriteBuffer &wboffset, int &offset, int v, int Smax){

	int empty = 0;
	wbtree.write(&v, 1);

	wboffset.write(&v, 1);
	wboffset.write(&Smax, 1);
	wboffset.write(&offset, 1);		


	if (CliTree == NULL)
		wbtree.write(&empty, 1);
	else {
		empty = 1;
		wbtree.write(&empty, 1);
		queue<CliNode*> q;
		q.push(CliTree);
		while(!q.empty()){
			CliNode *tp = q.front();
			q.pop();
			int key = tp->key;
			int lev = tp->lev;
			int nchi = tp->chi.size();
			wbtree.write(&key, 1);
			wbtree.write(&nchi, 1);
			wbtree.write(&lev, 1);
			for(int i=0; i<nchi; ++i){
				q.push(tp->chi[i]);
			}
			offset += 3;
		}
	}
	offset += 2;
	int t = -1;
	for(int i=0; i<GLOBAL_OFF; i++){
		wbtree.write(&t, 1);
	}
	offset += GLOBAL_OFF;

	wbtree.flush();
	wboffset.flush();
}

void Tree:: WriteTree2 (WriteBuffer &wbtree, int v){

	int empty = 0;
	wbtree.write(&v, 1);

	if (CliTree == NULL)
		wbtree.write(&empty, 1); 
	else {
		empty = 1;
		wbtree.write(&empty, 1);
		queue<CliNode*> q;
		q.push(CliTree);
		while(!q.empty()){
			CliNode *tp = q.front();
			q.pop();
			int key = tp->key;
			int lev = tp->lev;
			int nchi = tp->chi.size();
			wbtree.write(&key, 1);
			wbtree.write(&nchi, 1);
			wbtree.write(&lev, 1);
			for(int i=0; i<nchi; ++i){
				q.push(tp->chi[i]);
			}
		}
	}
	wbtree.flush();
}
#endif 
