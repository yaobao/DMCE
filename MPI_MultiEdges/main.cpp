#include "master_slave.h"

using namespace std;

int RANK, nslaves, master;
MPI_Status  *status;

int *order;
int nnum;
int nthreads;
int qv, thre;

int main(int argc, char **argv) {

	Runtimecounter rt;	
	rt.start();

	int nmach;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nmach);
	status = (MPI_Status*)malloc((nmach+1)*sizeof(MPI_Status));

	nslaves = --nmach;     
	master = 0;
	nnum = atoi(argv[2]); /*size of vertex*/
	order = ALLOC(MAX_V);

	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
	MPI_Barrier(MPI_COMM_WORLD);

	if(RANK == master) {
		FILE *gorder = fopen(argv[1], "rb");	/*order file*/
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

	generate();

	MPI_Barrier(MPI_COMM_WORLD);

	update(argv[3], argv[4]);
	MPI_Barrier(MPI_COMM_WORLD);
	
	free(order);
	free(status);
	MPI_Finalize();
	
	rt.stop();
	if(RANK==master)
		printf("%f\n", rt.GetRuntime()); // no more RANK=master

	return 0;
}
