#ifndef _SORTMERGE_H
#define _SORTMERGE_H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

typedef struct {
    char *nombreArchivo;
    unsigned int numLineasOrdenadas;
    char* lineaMasLarga;
    char* lineaMasCorta;
} stats_t;

typedef struct {
    char *nombreArchivo;
    stats_t *stats;
} thread_data_t;

typedef struct {
    FILE *archivo1;
    FILE *archivo2;
} merge_data_t;


// API's


#endif
