#include "multi-lookup.h"
#include "array.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>


void cmdlineErrorCheck(int argCount, char *i1, char*i2){
    if(argCount > (MAX_INPUT_FILES+5)){
        printf("Too many input files or extra command line arguments\n");
        exit(-1);
    }

    char *endptr;
    int num_requesters = strtol(i1, &endptr, 10);
    if(*endptr != '\0'){
        printf("Incorrect argument for number of requesters\n");
        exit(-1);
    }

    char *endptr2;
    int num_resolvers = strtol(i2, &endptr2, 10);
    if(*endptr2 != '\0'){
        printf("Incorrect argument for number of resolvers\n");
        exit(-1);
    }

    if(num_requesters > MAX_REQUESTER_THREADS){
        printf("Exceeded maximum of %d requester threads\n", MAX_REQUESTER_THREADS);
        exit(-1);
    }
    if(num_requesters < 1){
        printf("Need at least one requester thread\n");
        exit(-1);
    }
    if(num_resolvers > MAX_RESOLVER_THREADS){
        printf("Exceeded maximum of %d resolver threads\n", MAX_RESOLVER_THREADS);
        exit(-1);
    }
    if(num_resolvers < 1){
        printf("Need at least one resolver thread\n");
        exit(-1);
    }
}

void *requesterThread(void *arg){
    array *b = (array *)arg;
    int servicedFiles = 0;
    while(1){
        sem_wait(&b->argvAccess);
            if(b->argVector[b->argvIndex] == NULL){
                sem_post(&b->argvAccess);
                break;
            }
            FILE *currFile = fopen(b->argVector[b->argvIndex], "r");
            b->argvIndex++;
        sem_post(&b->argvAccess);
        if(!currFile){
            sem_wait(&b->printing);
                printf("Input file could not be opened\n");
            sem_post(&b->printing);
            continue;
        }
        char *buffer = malloc(MAX_NAME_LENGTH);
        if(!buffer){
            sem_wait(&b->printing);
                printf("Buffer malloc failed in requester thread\n");
            sem_post(&b->printing);
            fclose(currFile);
            pthread_exit(NULL);
        }
        while(fgets(buffer, MAX_NAME_LENGTH, currFile)!=NULL){ //process lines from file
            int length = strlen(buffer);
            if(length > 0 && buffer[length-1] =='\n'){
                buffer[length-1] = '\0'; //does the dns lookup file expect a null terminated string?
            }
            char *copy = strdup(buffer);
            if(!copy){
                sem_wait(&b->printing);
                    printf("strdup of buffer failed in requester thread\n");
                sem_post(&b->printing);
                break;
            }
            array_put(b, copy);
            sem_wait(&b->loggingServiced);
                fprintf(b->serviced, "%s\n", buffer);
            sem_post(&b->loggingServiced);
        }
        fclose(currFile);
        free(buffer);
        servicedFiles++;
    }
    pthread_t threadID = pthread_self();
    sem_wait(&b->printing);
        printf("thread %lu serviced %d files\n", (unsigned long)threadID, servicedFiles);
    sem_post(&b->printing);
    pthread_exit(NULL);
}

void *resolverThread(void *arg){
    array *b = (array *)arg;
    int resolvedHosts = 0;

    while(1){
        char *host;
        array_get(b, &host);
        if(host == NULL){
            pthread_t threadID = pthread_self();
            sem_wait(&b->printing);
                printf("thread %lu resolved %d hosts\n", (unsigned long)threadID, resolvedHosts);
            sem_post(&b->printing);
            free(host);
            pthread_exit(NULL);
        }
        //do DNS resolution
        char IPstr[MAX_IP_LENGTH];
        if((dnslookup(host, IPstr, MAX_IP_LENGTH))!= UTIL_SUCCESS){
            strncpy(IPstr, "NOT_RESOLVED", MAX_IP_LENGTH);
        }
        sem_wait(&b->loggingResults);
            fprintf(b->results, "%s, %s\n", host, IPstr);
        sem_post(&b->loggingResults);
        free(host);
        resolvedHosts++;
    }
}

int main(int argc, char *argv[]){ //synchronize boundedBuffer, logfiles, stdout/stderr, argc/argv
    struct timeval start;
    struct timeval end;
    long seconds;
    long microseconds;
    double elapsed;
    gettimeofday(&start, NULL);

    if(argc < 6){
        printf("Too few command line arguments\n");
        exit(-1);
    }
    cmdlineErrorCheck(argc, argv[1], argv[2]);

    int num_requesters = atoi(argv[1]);
    int num_resolvers = atoi(argv[2]);
    pthread_t requesters[num_requesters];
    pthread_t resolvers[num_resolvers];
    
    char *toService = argv[3];
    if(access(toService, F_OK)==0){
        if(access(toService, W_OK)!=0){
            printf("Given log file is not writable\n");
            exit(-1);
        }
    }
    char *toResolve = argv[4];
    if(access(toResolve, F_OK)==0){
        if(access(toResolve, W_OK)!=0){
            printf("Given log file is not writable\n");
            exit(-1);
        }
    }


    array boundedBuffer;
    if(array_init(&boundedBuffer) < 0){
        printf("Bounded buffer init failed");
        exit(-1);
    }
    boundedBuffer.serviced = fopen(toService, "w");
    if(!boundedBuffer.serviced){
        printf("Failed to open serviced file\n");
        exit(-1);
    }
    boundedBuffer.results = fopen(toResolve, "w");
    if(!boundedBuffer.results){
        printf("Failed to open results file\n");
        exit(-1);
    }
    boundedBuffer.argVector = argv;

    for(int i = 0; i < num_requesters; i++){
        if(pthread_create(&requesters[i], NULL, requesterThread, (void*)&boundedBuffer) != 0){
            printf("Failed to create requester thread\n");
            exit(-1);
        }
    }

    for(int i = 0; i < num_resolvers; i++){
        if(pthread_create(&resolvers[i], NULL, resolverThread, (void*)&boundedBuffer) != 0){
            printf("Failed to create resolver thread\n");
            exit(-1);
        }
    }


    for(int i = 0; i < num_requesters; i++){
        pthread_join(requesters[i], NULL);
    }

    //send poison pills to resolvers
    for(int i = 0; i < num_resolvers; i++){
        array_put(&boundedBuffer, NULL);
    }

    for(int i = 0; i < num_resolvers; i++){
        pthread_join(resolvers[i], NULL);
    }


    array_free(&boundedBuffer);
    gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    if(microseconds < 0){
        seconds --;
        microseconds += 1000000;
    }
    elapsed = seconds + microseconds*1e-6;
    printf("total time is %.6f seconds\n", elapsed);
    exit(0);
}