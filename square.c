/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program constructs no-touch squarings of a given square. 
 * 
 * 
 * Compile with:
 *     
 *     cc -o square -O4 square.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

typedef int boolean;

#define FALSE 0
#define TRUE  1

typedef unsigned char size;
#define MAX_SIZE 100

size areaSize;
size grid[MAX_SIZE][MAX_SIZE] = {{0}};

#define SQUARE(X,Y) (grid[Y][X])

#define SET_SQUARE(X,Y,S) {int local_i, local_j; \
                           for(local_i = (X); local_i < (X) + (S); local_i++)\
                               for(local_j = (Y); local_j < (Y) + (S); local_j++)\
                                   grid[local_j][local_i] = (S);\
                          }

#define UNSET_SQUARE(X,Y,S) {int local_i, local_j; \
                             for(local_i = (X); local_i < (X) + (S); local_i++)\
                                 for(local_j = (Y); local_j < (Y) + (S); local_j++)\
                                     grid[local_j][local_i] = 0;\
                            }

#define LIES_IN_SQUARE(SX,SY,S,X,Y) (((X) >= (SX)) && ((Y) >= (SY)) && ((X) < (SX + S)) && ((Y) < (SY + S)))

#define LIES_IN_MAINSQUARE(X,Y) (((X) < areaSize) && ((Y) < areaSize))

//=============================EXPORT======================================

void printSquare(FILE *f){
    int i, j;
    for(i = 0; i < areaSize; i++){
        for(j = 0; j < areaSize; j++){
            fprintf(f, "%d", grid[i][j]);
        }
        fprintf(f, "\n");
    }
}
//==============================USAGE======================================

void help(char *name) {
    fprintf(stderr, "The program %s constructs no-touch squarings of a given square.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n\n\n", name);
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] n\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

int main(int argc, char *argv[]) {

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
         {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    
    if(argc - optind != 1){
        fprintf(stderr, "Illegal number of arguments.\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    areaSize = (size)atoi(argv[optind]);
    if(areaSize > MAX_SIZE){
        fprintf(stderr, "Squares of that size are not supported.\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
