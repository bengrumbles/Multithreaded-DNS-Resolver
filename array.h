#ifndef ARRAY_H
#define ARRAY_H

#include <semaphore.h>
#include <stdio.h>
#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 20 //Max length name in the zip file was 16. I just went with 20
typedef struct {
    char *array[ARRAY_SIZE]; //array of 8 pointers
    int front;
    int back;
    sem_t empty;
    sem_t items_avail;
    sem_t mutex;

    int argvIndex;
    char **argVector;
    FILE *serviced;
    FILE *results;
    sem_t argvAccess; //use this for updating argvindex and accessing argvector
    sem_t loggingServiced; //access to file where serviced files are tracked
    sem_t loggingResults; //access to file where results of DNS res are tracked
    sem_t printing; //for access to stdout/stderr
} array;


int  array_init(array *s);                   // initialize the array
int  array_put (array *s, char *hostname);   // place element into the array, block when full
int  array_get (array *s, char **hostname);  // remove element from the array, block when empty
void array_free(array *s);                   // free the array's resources

#endif