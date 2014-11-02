#include "master_slave.h"

using namespace std;

int RANK, nblk, r, nslaves, master;
MPI_Status  *status;

int *order;
int nnum;
int nthreads;
int qv, thre;

/*mpirun -np m ./parmce graph.bin blocknum order.bin vertexnum s e type(delete, insert)*/
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
			order[v] = vo;
		}	
		fclose(gorder);
	}


	MPI_Bcast(order, nnum, MPI_INT, master, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	generate(argv[1]);
	MPI_Barrier(MPI_COMM_WORLD);

	/*
	 * argv[5]: s;
	 * argv[6]: e; (order[s] < order[e]);
	 * argv[7]: update_type; (0: insert; 1, delete);
	 */
	update(argv[5], argv[6], argv[7]);
	MPI_Barrier(MPI_COMM_WORLD);

	free(status);
	free(order);
	MPI_Finalize();

	rt.stop();
	if(RANK == master)
		printf("%f\n", rt.GetRuntime());
	
	return 0;
}
