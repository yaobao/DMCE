#include "master_slave.h"

using namespace std;

int RANK, nslaves, master;
MPI_Status  *status;

int *order;
int nnum;
int nthreads;
int qv, thre;

/*mpirun -np 2 ./parmce newgoogle.bin 1 core.bin 875713*/
int main(int argc, char **argv) {

	Runtimecounter rt;	
	rt.start();

	int nmach;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nmach);
	status = (MPI_Status*)malloc((nmach+1)*sizeof(MPI_Status));

	nslaves = --nmach;                             
	master = 0;
	nnum = atoi(argv[3]); /*size of vertex*/
	order = ALLOC(MAX_V);

	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
	MPI_Barrier(MPI_COMM_WORLD);

	if(RANK == master) {
		FILE *gorder = fopen(argv[2], "rb");	/*order file*/
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

	generate(argv[1]);	//argv[1]: inDir of part files in hdfs

	MPI_Barrier(MPI_COMM_WORLD);

	update(argv[4], argv[5]);	//argv[4]: file of inserted edges or deleted files;
								// argv[5]: types of insertion or deletion

	MPI_Barrier(MPI_COMM_WORLD);
	
	free(order);
	free(status);
	MPI_Finalize();
	
	rt.stop();
	if(RANK==master)
		printf("%f\n", rt.GetRuntime()); // no more RANK=master

	return 0;
}
