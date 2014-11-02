#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <mpi.h>
#include <queue>

using namespace std;

const int MAX_B = 4;
const int MAX_V = 3000000;
const int MAX_E = 100000000;
const int GLOBAL_OFF = 300;			//global_offset for the update tree
const int NCH_OFF = 100;			//improve later

typedef vector<int> VI;
typedef bitset<MAX_V> BS;
typedef long long LL;

extern int RANK, master;
extern int nslaves;
extern MPI_Status  *status;

#define ALLOC(N) (int*)malloc((N)*sizeof(int))

enum {
	utag,
	nlttag,
	ngttag,
	adjtag,
	ntag,
	rettag,
	flagtag,
	blktag,
	qvtag,
	degtag,
	endtag,
	ordertag,
	lentag,
	slentag,
	nametag
};

extern FILE *treeFiles[MAX_B];
extern FILE *offsetFiles[MAX_B];
extern FILE *blkFiles[MAX_B];
extern int *order;
extern int nnum;

void outputSet(int *set, int nset) {
	for(int i=0; i<nset; ++i)
		cout << set[i] << " ";
	cout << endl;
}

int binarysearch(int *p, int length, int v){
	int low = 0, high = length;
	int mid = (low+high)/2;
	while(low < high){
		mid = (low+high)/2;
		if(p[mid] == v)
			break;
		else if(p[mid] < v)
			low = mid+1;
		else
			high = mid;
	}
	return mid;
}

int * combine(int *edge, int ngt, int nlt, int *start) {
	int i=0, j=ngt, k=0;
	while(i<ngt && j<ngt+nlt) {
		if(start[i]<start[j])
			edge[k++] = start[i++];
		else
			edge[k++] = start[j++];

	}
	if(i==ngt) {
		while(j<ngt+nlt)    edge[k++] = start[j++];
	}
	else {
		while(i<ngt)    edge[k++] = start[i++];

	}
	return edge;

}

#endif /* GLOBAL_H_ */
