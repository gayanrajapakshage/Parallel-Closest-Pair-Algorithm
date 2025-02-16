#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "point.h"
#include "utilities_closest.h"
#include "serial_closest.h"
#include "parallel_closest.h"


void print_usage() {
    fprintf(stderr, "Usage: closest -f filename -d pdepth\n\n");
    fprintf(stderr, "    -d Maximum process tree depth\n");
    fprintf(stderr, "    -f File that contains the input points\n");

    exit(1);
}

int main(int argc, char **argv) {
    int n = -1;
    long pdepth = -1;
    char *filename = NULL;
    int pcount = 0;

    // TODO: Parse the command line arguments
    
    // Report if # of arguments isn't 5
    if (argc != 5) {
        print_usage();
    }

    int ch;
    // Loop over command line arguments
    while ((ch = getopt(argc, argv, "f:d:")) != -1) {
        switch (ch) {
        case 'f': // Assign filename argument to var
            filename = optarg;
            break;
        case 'd': // Assign pdepth argument to var
            pdepth = strtol(optarg, NULL, 10);
            break;
        case '?': // Report if option other than -f and -d is encountered
            print_usage();
        default: // Report for other unexpected behaviour
            print_usage();
        }
    }

    // Report if additional, unexpected args provided
    if (optind < argc) {
        print_usage();
    }

    // Adjust argc and argv
    argc -= optind;
    argv += optind;

    // Report if filename or pdepth aren't correctly assigned
    if (filename == NULL || pdepth == -1) {
        print_usage();
    }

    // Read the points
    n = total_points(filename);
    struct Point points_arr[n];
    read_points(filename, points_arr);

    // Sort the points
    qsort(points_arr, n, sizeof(struct Point), compare_x);

    // Calculate the result using the parallel algorithm.
    double result_p = closest_parallel(points_arr, n, pdepth, &pcount);
    printf("The smallest distance: is %.2f (total worker processes: %d)\n", result_p, pcount);

    exit(0);
}
