#include "master_slave.h"

using namespace std;

int RANK, nblk, r, nslaves, master;
MPI_Status  *status;

int *order;
int nnum;
int nthreads;
int qv, thre;

/* mpirun -np m ./parmce graph.bin blocknum order.bin vertexnum type(none, update, query)
 * none(0);
 * update(1) s e type(delete, insert);
 * query(2) type(v, threshold, Set),
 * v(0),
 * threshold(1),
 * Set(2);
 */
int main(int argc, char **argv) {

	Runtimecounter rt;	
	rt.start();
  
	int nmach;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nmach);
	status = (MPI_Status*)malloc((nmach+1)*sizeof(MPI_Status));
	
	nslaves = --nmach;                            
   	master = 0;
	r = atoi(argv[2]);                             //number of blocks in each slave
	nnum = atoi(argv[4]); /*size of vertex*/
	nblk = r*nslaves;
	order = ALLOC(MAX_V);

	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(RANK == master) {
		FILE *gorder = fopen(argv[3], "rb");	/*order file*/
		ReadBuffer rborder(gorder);
		int v, vo;     
		while(!rborder.isend) {
			rborder.read(&vo);
			rborder.read(&v);
		//	order[v] = vo;
			order[vo] = v;
		}	
		fclose(gorder);
	//	outputSet(order, nnum);
	}

	MPI_Bcast(order, nnum, MPI_INT, master, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG
	cout << "to generate " << argv[1] << endl; 
#endif
	generate(argv[1]);
	MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG
	cout << "generated " << argv[1] << endl; 
#endif

	/*
	 * argv[5]: s;
	 * argv[6]: e; (order[s] < order[e]);
	 * argv[7]: update_type; (0: insert; 1, delete);
	 */
	int type = atoi(argv[5]);
	if (type == 1)
		update(argv[6], argv[7], argv[8]);
	else if(type == 2){
		int query_type = atoi(argv[6]);
		if (query_type == 0)
			queryV(argv[7], argv[8]);
		else if (query_type == 1)
			queryThre(argv[7], argv[8]);
		else if (query_type == 2)
			querysupSet(argv[7], argv[8], argv[1]);
		else if (query_type == 3)
			querysubSet(argv[7], argv[8]);	
	}
	MPI_Barrier(MPI_COMM_WORLD);
	free(status);
	free(order);
	MPI_Finalize();

	rt.stop();
	if(RANK == master)
		printf("%f\n", rt.GetRuntime());
	
	return 0;
}
