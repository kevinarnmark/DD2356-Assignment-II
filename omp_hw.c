#include "omp.h"
#include <stdio.h>
int main(int argc, char *argv[]) 
{
    //omp_set_num_threads(8);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        //int np = omp_get_num_threads();
        printf("Hello World from Thread %d!\n", id);
    }
    return 0;
}
