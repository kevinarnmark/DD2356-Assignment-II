#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

void generate_random(double *input, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    input[i] = rand() / (double)(RAND_MAX);
  }
}

typedef struct { double l_sum; char pad[128]; } p_double; 

double opt_local_sum(double *x, size_t size, int n_threads) {
    p_double local_sum[n_threads];
    #pragma omp parallel shared(local_sum)
    {
        int id = omp_get_thread_num();
        local_sum[id].l_sum = 0.0;
        #pragma omp for
            for (size_t i = 0; i<size; i++) {
                local_sum[id].l_sum += x[i];
            }
    }
    double sum_val = 0.0;
    for (int i = 0; i < n_threads; i++) {
       sum_val += local_sum[i].l_sum;
    }
    return sum_val;
}

double omp_local_sum(double *x, size_t size, int n_threads) {
    double local_sum[n_threads];
    #pragma omp parallel shared(local_sum)
    {
        int id = omp_get_thread_num();
        #pragma omp for
            for (size_t i = 0; i<size; i++) {
                local_sum[id] += x[i];
            }
    }
    double sum_val = 0.0;
    for (int i = 0; i < n_threads; i++) {
       sum_val += local_sum[i];
    }
    return sum_val;
}

double omp_sum(double *x, size_t size)
{
    double sum_val = 0.0;
    #pragma omp parallel for
        for (size_t i = 0; i < size; i++) {
            sum_val += x[i];
        }
    return sum_val;
}

double omp_critical_sum(double *x, size_t size)
{
    double sum_val = 0.0;
    #pragma omp parallel for
        for (size_t i = 0; i < size; i++) {
            #pragma omp critical 
            {
                sum_val += x[i];
            }
        }
    return sum_val;
}

double serial_sum(double *x, size_t size)
{
    double sum_val = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum_val += x[i];
    }
    return sum_val;
}

int main(int argc, char *argv[]){
    double *array;
    double start_time, avg_time, result;
    int n_tests = 10;
    int n_threads = 10;
    int threads[] = {1, 2, 4, 8, 12, 16, 20, 24, 28, 32};
    int size = 1E+8;
    double time[n_tests], thread_time[n_threads], thread_std[n_threads];
    array = (double *) malloc(size*sizeof(double));
    
    generate_random(array, size);
    
    for (int a=0; a<n_threads; a++) {
    omp_set_num_threads(threads[a]);
    
    result = opt_local_sum(array, size, threads[a]); //Avoiding cold start
    
    start_time = omp_get_wtime();
    for (int i=0; i<n_tests; i++) {
        result = opt_local_sum(array, size, threads[a]);
        time[i] = omp_get_wtime() - start_time;
    }
    avg_time = (omp_get_wtime() - start_time) / n_tests;
    
    //Calculate std deviation
    double sd_sum = 0;
    for (int i=0; i<n_tests-1; i++) {
        //printf("Time: %d, %.15f \n", i, (time[i+1] - time[i]));
        sd_sum += pow((time[i+1] - time[i]) - avg_time, 2);
    }
    
    double std_dev = sqrt((sd_sum/(n_tests - 1)));

    printf("Average execution time of serial sum with size %d was %fs with a std deviation of %.15f \n", size, avg_time, std_dev);
    printf("Control: Serial sum = %f, OMP sum = %f \n", serial_sum(array, size), result);
    
    thread_time[a] = avg_time;
    thread_std[a] = std_dev;
    }
    printf("Threads Time Stdev\n");
    for (int a=0;a < n_threads; a++) {
        printf("%d    %.15f    %.15f \n", threads[a], thread_time[a], thread_std[a]);
    }
    free(array);
}
