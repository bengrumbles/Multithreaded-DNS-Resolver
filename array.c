#include "array.h"
#include <semaphore.h>
#include <string.h>


int  array_init(array *s){
    sem_init(&s->empty, 0, ARRAY_SIZE);
    sem_init(&s->items_avail, 0, 0);
    sem_init(&s->mutex, 0, 1);
    s->front = 0;
    s->back = 0;

    sem_init(&s->argvAccess, 0, 1);
    sem_init(&s->loggingServiced, 0, 1);
    sem_init(&s->loggingResults, 0, 1);
    sem_init(&s->printing, 0, 1);
    s->argvIndex = 5;
    return 0;
}

int  array_put (array *s, char *hostname){
    sem_wait(&s->empty);
    sem_wait(&s->mutex);
    s->array[s->back] = hostname;
    s->back = (s->back +1) % ARRAY_SIZE;
    sem_post(&s->mutex);
    sem_post(&s->items_avail);
    return 0;
}

int  array_get (array *s, char **hostname){
    sem_wait(&s->items_avail);
    sem_wait(&s->mutex);
    *hostname = s->array[s->front];
    s->front = (s->front +1) % ARRAY_SIZE;
    sem_post(&s->mutex);
    sem_post(&s->empty);
    return 0;
}

void array_free(array *s){
    sem_destroy(&s->empty);
    sem_destroy(&s->items_avail);
    sem_destroy(&s->mutex);

    sem_destroy(&s->argvAccess);
    sem_destroy(&s->loggingServiced);
    sem_destroy(&s->loggingResults);
    sem_destroy(&s->printing);

    fclose(s->results);
    fclose(s->serviced);
}