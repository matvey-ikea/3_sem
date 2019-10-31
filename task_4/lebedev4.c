#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

double monte(double xmax, double ymax, int nsteps, int seed) {
    int pcount = 0;
    struct drand48_data rand_data;
    srand48_r(seed, &rand_data);
    for(int i = 0; i < nsteps; i++) {
        double x, y;
        drand48_r(&rand_data, &x);
        drand48_r(&rand_data, &y);
        pcount += y < (x - x*x);
    }
    return xmax * ymax * pcount;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define nthreads 8
double accumulator = 0;
int a = 0x20000;
int npoints = 0x20000000;
void *routine() {
    double delta = monte(1, 1, npoints / nthreads, rand());
    pthread_mutex_lock(&mutex);
    accumulator += delta / npoints;
    pthread_mutex_unlock(&mutex);
}
int main() {
    pthread_t threads[nthreads];
    for(long long i = 0; i < nthreads; i++)
        pthread_create(&threads[i], NULL, routine, NULL);
    for(long long i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);
}