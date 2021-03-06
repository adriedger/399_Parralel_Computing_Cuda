// Andre Driedger 1805536
// CMPT399 A1 Matrix Product

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <xmmintrin.h>
#include <pmmintrin.h>


//int A[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//int B[] = {1, 0, 0, 0 ,1 ,0 ,0 ,0, 1};
//int AB[9];
//double mSize = sqrt(sizeof(A)/sizeof(A[0]));

double *A; 
double *B;
double *AB;
int LENGTH, THRDS;

double *makeRandSet(int size, int seed){
	double *set;
	int i;
	srand(seed);
	set = malloc(((size*size) * sizeof(double)));
	if(set == NULL)
		return NULL;
	for(i=0; i<(size*size); i++)
		set[i] = rand() % 5;
	return set;
}

void *matrixMulti(void *t){
	int tid = (long)t;
	int x, y, i;

	int regular_passes = (LENGTH*LENGTH)/THRDS;

	int A_Row = tid / LENGTH;
	int B_Row = tid % LENGTH;
	int index = tid;

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);
	
	for(i=0; i<regular_passes+1; i++){
		if(index < LENGTH*LENGTH){
//			int B_Row = index % LENGTH;
			for(x=A_Row*LENGTH, y=B_Row*LENGTH; x<(A_Row+1)*LENGTH; x++, y++)
				AB[index] += A[x] * B[y];
			
			index += THRDS;
			A_Row = index / LENGTH;
			B_Row = index % LENGTH;
		}
	}
/*
	int row, col;
	for(){
		for()
			AB[row*LENGTH + col] = dotProductSSE(row, col);
	}
*/
	clock_gettime(CLOCK_REALTIME, &end);
	printf("Kernel time is %f\n", (double)((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)*1e-9));

	pthread_exit(NULL);
}

float dotProductSSE(int row, int col) {
	float prod = 0.0, tmp;
  	int i;
  	__m128 x, y, z;

  	for(i=0; i<LENGTH; i+=4) {
    		x = _mm_load_ps(&A[row*LENGTH + i]);
   		y = _mm_load_ps(&B[col*LENGTH + i]);
    		z = _mm_mul_ps(x, y);

   	 x = _mm_hadd_ps(x, x);
  	 x = _mm_hadd_ps(x, x);
   	 _mm_store_ss(&tmp, x);
    	prod += tmp;
  	}

 	return prod;
}
	

int main(int argc, char *argv[]){
	
	int i;
	LENGTH = atoi(argv[argc-2]);
	THRDS = atoi(argv[argc-1]);

	A = makeRandSet(LENGTH, time(NULL) - 1);	
	B = makeRandSet(LENGTH, time(NULL));
	AB = malloc((LENGTH*LENGTH) * sizeof(double));
	
	if(!strcmp(argv[argc-3], "-O")){
		for(i=0; i<LENGTH*LENGTH; i++){
			printf("%f ", A[i]);
			if(i % LENGTH == LENGTH-1)
				printf("\n");
		}
		printf("\n");
		for(i=0; i<LENGTH*LENGTH; i++){
			printf("%f ", B[i]);
			if(i % LENGTH == LENGTH-1)
				printf("\n");
		}
		printf("\n");
		
	}


	pthread_t threads[THRDS];
	pthread_attr_t attr;
	int rc;
	long t;
	void *status;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for(t=0; t<THRDS; t++){
		rc=pthread_create(&threads[t], NULL, matrixMulti, (void*)t);
		if(rc){
			printf("ERROR: RETURN CODE FROM PTHREAD_CREATE() IS %d\n", rc);
			exit(-1);
		}
	}
	pthread_attr_destroy(&attr);
	for(t=0; t<THRDS; t++){
		rc = pthread_join(threads[t], &status);
		if(rc){
			printf("ERROR: RETURN CODE FROM PTHREAD_JOIN() IS %d\n", rc);
			exit(-1);
		}
	}
	
	printf("\n");
	if(!strcmp(argv[argc-3], "-O")){
		for(i=0; i<LENGTH*LENGTH; i++){
			printf("%f ", AB[i]);
			if(i % LENGTH == LENGTH-1)
				printf("\n");
		}
		printf("\n");
	}
	
	free(A);
	free(B);
	free(AB);

	pthread_exit(NULL);
}
