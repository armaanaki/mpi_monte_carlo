#include <mpi.h>
#include <cstdio>
#include <stdlib.h>
#include <random>

double doubleRand(double min, double max);

int main(int argc, char** argv) {
    int my_rank, comm_sz;

    // init MPI variables
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    long long total_tosses, my_hits = 0, total_hits = 0;

    // get the total tosses
    bool error = false;

    if (my_rank == 0) {
        if (argc != 2) {
            error = true;
            printf("you must have total tosses as a command arg and nothing else!");
        } else total_tosses = strtol(argv[1], NULL, 10); 

        if (total_tosses % comm_sz != 0) {
            error = true;
            printf("the total tosses must be divisible by total processes!");
        }
    }


    // if these was an error, distribute it and terminate all
    MPI_Bcast(&error, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
    if (error) {
        MPI_Finalize();
        exit(-1);
    }

    // broadcat the total tosses
    MPI_Bcast(&total_tosses, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    // setup the tosses for each process
    long long my_tosses = total_tosses / comm_sz;

    for (long long i = 0; i < my_tosses; i++) {
        double x = doubleRand(-1, 1);
        double y = doubleRand(-1, 1);

        double distance_squared = x*x + y*y;
        if (distance_squared <= 1) my_hits++;
    }

    // add all hits to one variable
    MPI_Reduce(&my_hits, &total_hits, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // print the pi estimate if you are the first thread
    if (my_rank == 0) {
        double pi_estimate = 4*total_hits/(double) total_tosses;
        printf("pi guess: %.6f\n", pi_estimate);
   }

    MPI_Finalize();
}

    
/*
 * Function: 	doubleRand
 * Purpose:	 	generate a random number in a threadsafe fashion
 * In args:		min, max
 * DISCLAIMER:	this solution was found from the following: https://stackoverflow.com/questions/29709897/c-thread-safe-uniform-distribution-random-number-generation
 */
double doubleRand(double min, double max) {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}
