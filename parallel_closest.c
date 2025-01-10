#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "parallel_closest.h"
#include "utilities_closest.h"


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {
    // Base Case
    if (n < 4 || pdmax == 0){
        return closest_serial(p, n);
    }
    
    // Split points array into two halves
    int mid = n / 2;
    struct Point *left = p;
    struct Point *right = p + mid;
    
    // Create pipe1 for first child process
    int pipe1[2];
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(1);
    }

    // Fork first child
    int r1 = fork();
    
    // Obtain and send distance of closest points on LHS to parent process through pipe1
    if (r1 == 0){
        close(pipe1[0]);
        int curr_pcount = 0;
        double left_closest = closest_parallel(left, mid, pdmax - 1, &curr_pcount);
        if (write(pipe1[1], &left_closest, sizeof(left_closest)) == -1){
            perror("write to pipe1");
            exit(1);
        }
        close(pipe1[1]);
        exit(curr_pcount);
    }

    // Handle fork failure
    else if (r1 < 0){
        perror("fork");
        exit(1);
    }

    // Create pipe2 for second child process
    int pipe2[2];
    if (pipe(pipe2) == -1) {
        perror("pipe2");
    }

    // Fork second child
    int r2 = fork();
    
    // Obtain and send distance of closest points on RHS to parent process through pipe2
    if (r2 == 0){
        close(pipe2[0]);
        int curr_pcount = 0;
        double right_closest = closest_parallel(right, n - mid, pdmax - 1, &curr_pcount);
        if (write(pipe2[1], &right_closest, sizeof(right_closest)) == -1){
            perror("write to pipe2");
            exit(1);
        }
        close(pipe2[1]);
        exit(curr_pcount);
    }

    // Handle fork failure
    else if (r2 < 0){
        perror("fork");
        exit(1);
    }

    // Close write ends of the pipes
    close(pipe1[1]);
    close(pipe2[1]);
    
    // Wait for child processes to complete
    int status1, status2;
    if (wait(&status1) == -1 || wait(&status2) == -1)  {
        perror("wait");
        exit(1);
    }

    // Check exit statuses of child processes and update pcount
    if (WIFEXITED(status1) && WIFEXITED(status2)) {
        *pcount += WEXITSTATUS(status1) + WEXITSTATUS(status2) + 2; // Add 2 for the two children
    } else {
        perror("Child exited abnormally");
        exit(1);
    }
    
    // Read distances from pipes
    double left_closest;
    double right_closest;
    read(pipe1[0], &left_closest, sizeof(left_closest));
    read(pipe2[0], &right_closest, sizeof(right_closest));
    
    // Close read ends of the pipes
    close(pipe1[0]);
    close(pipe2[0]);


    /* The following is a modified portion of code from the
    closest_serial function in serial_closest.c. It is used to
    find the distance between the closest pair of points where 
    one point is on the LHS and the other point is on the RHS.
    The minimum of the three found distances is returned.*/

    // Find the smaller of LHS and RHS distances 
    double d = min(left_closest, right_closest);

    // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point.
    struct Point *strip = malloc(sizeof(struct Point) * n);
    if (strip == NULL) {
        perror("malloc");
        exit(1);
    }

    int j = 0;
    for (int i = 0; i < n; i++) {
        if (abs(p[i].x - p[mid].x) < d) {
            strip[j] = p[i], j++;
        }
    }

    // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
    double new_min = min(d, strip_closest(strip, j, d));
    free(strip);

    return new_min;
}

