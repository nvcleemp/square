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

unsigned int solutionCount = 0;

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
    fprintf(f, "\n");
}

void tikzSquare(FILE *f){
    int i, j;
    
    fprintf(f, "\\begin{tikzpicture}["
                             "yscale=-1,"
                             "s1/.style={red},"
                             "s2/.style={green},"
                             "s3/.style={blue},"
                             "s4/.style={yellow},"
                             "s5/.style={cyan},"
                             "s6/.style={magenta}"
                "]\n\n");
    
    fprintf(f, "\\fill[s%d] (0,0) -- (%d,0) -- (%d,%d) -- (0,%d) -- (0,0);\n",
            SQUARE(0,0), SQUARE(0,0), SQUARE(0,0), SQUARE(0,0), SQUARE(0,0));
    
    for(i = 1; i < areaSize; i++){
        if(SQUARE(i,0) != SQUARE(i-1,0)){
            fprintf(f, "\\fill[s%d] (%d,0) -- (%d,0) -- (%d,%d) -- (%d,%d) -- (%d,0);\n",
            SQUARE(i,0), i, i + SQUARE(i,0), i + SQUARE(i,0), SQUARE(i,0), i, SQUARE(i,0), i);
        }
    }
    
    for(i = 1; i < areaSize; i++){
        if(SQUARE(0,i) != SQUARE(0, i-1)){
            fprintf(f, "\\fill[s%d] (0,%d) -- (%d,%d) -- (%d,%d) -- (0,%d) -- (0,%d);\n",
            SQUARE(0,i), i, SQUARE(0,i), i, SQUARE(0,i), i + SQUARE(0,i), i + SQUARE(0, i), i);
        }
        for(j = 1; j < areaSize; j++){
            if((SQUARE(j,i) != SQUARE(j, i-1)) && (SQUARE(j,i) != SQUARE(j-1, i))){
                fprintf(f, "\\fill[s%d] (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d);\n",
                        SQUARE(j,i),
                        j, i,
                        j + SQUARE(j,i), i,
                        j + SQUARE(j,i), i + SQUARE(j, i),
                        j, i + SQUARE(j, i),
                        j, i);
            }
        }
    }
    
    fprintf(f, "\n\\end{tikzpicture}\n");
}

void handleFinishedSquare(){
    solutionCount++;
    printSquare(stderr);
}

void addNextSquare(int lastX, int lastY){
    //first we look for the next free position
    int x, y;
    x = lastX;
    y = lastY;
    while(x < areaSize && SQUARE(x,y)){
        x++;
    }
    if(x == areaSize){
        for(y = lastY + 1; y < areaSize; y++){
            x = 0;
            while(x < areaSize && SQUARE(x,y)){
                x++;
            }
            if(x < areaSize){
                break;
            }
        }
    }
    
    if(x == areaSize && y == areaSize){
        //square is completely filled
        handleFinishedSquare();
        return;
    }
    
    if(areaSize - x < 3 || areaSize - y < 3){
        //no squares of size 1 or 2 in the outer 3 layers
        return;
    }
    
    size minSize = 1, maxSize = 0;
    
    if(x < 3 || y < 3) {
        //no squares of size 1 or 2 in the outer 3 layers
        minSize = 3;
        
        if(x == 0 && y == 0){
            //no square of size 3 in the corners
            minSize = 4;
        }
    }
    
    while(maxSize < 6 && LIES_IN_MAINSQUARE(x + maxSize, y + maxSize) &&
            (!SQUARE(x + maxSize, y))){
        maxSize++;
    }
    
    size s;
    for(s = minSize; s <= maxSize; s++){
        if(y - 1 >= 0){
            int i = 0;
            while(i < s && SQUARE(x + i, y - 1) != s) {
                i++;
            }
            if(i < s){
                continue;
            }
        }
        if(x - 1 >= 0){
            if(SQUARE(x - 1, y) == s){
                continue;
            }
        }
        if(x + s + 1 < areaSize){
            if(SQUARE(x + s + 1, y) == s){
                continue;
            }
        }
        SET_SQUARE(x, y, s);
        addNextSquare(x, y);
        UNSET_SQUARE(x, y, s);
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
    
    addNextSquare(0,0);
    
    fprintf(stderr, "Found %d solution%s.\n", 
            solutionCount, solutionCount == 1 ? "" : "s");
    
    return EXIT_SUCCESS;
}
