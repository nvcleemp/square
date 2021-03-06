/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2015 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

/* This program constructs no-touch or nowhere-neat square tilings of a given
 * rectangle. 
 * 
 * 
 * Compile with:
 *     
 *     cc -o st -O4 st.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

const char const * colours[]= {"", //no zero
                               "red", //1
                               "blue", //2
                               "yellow", //3
                               "green", //4
                               "cyan", //5
                               "brown!80!white", //6
                               "violet!60!white", //7
                               "magenta", //8
                               "pink"}; //9 
#define COLOUR_NAMES_COUNT 9

typedef int boolean;

#define FALSE 0
#define TRUE  1

boolean tikzOutput = FALSE;
FILE *tikzOutputFile = NULL;

boolean skipCanonicityCheck = FALSE;

typedef unsigned char size;
#define MAX_SIZE 100

size areaWidth;
size areaHeight;
size grid[MAX_SIZE][MAX_SIZE] = {{0}};
size corners[MAX_SIZE][MAX_SIZE] = {{0}};

size smallestTile = 1;
size largestTile = 6;

unsigned int solutionCount = 0;

//profiling variables
unsigned int maxTiles = 0;
unsigned int currentTiles = 0;
unsigned int maxReachedHeight = 0;

#define GRID_VALUE(X,Y) (grid[Y][X])

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

#define UPPERLEFT 1
#define LOWERLEFT 2
#define UPPERRIGHT 3
#define LOWERRIGHT 4
#define IS_UPPERLEFT_CORNER(X,Y) (corners[Y][X]==UPPERLEFT)
#define IS_LOWERLEFT_CORNER(X,Y) (corners[Y][X]==LOWERLEFT)
#define IS_UPPERRIGHT_CORNER(X,Y) (corners[Y][X]==UPPERRIGHT)
#define SET_SQUARE_CORNERS(X,Y,S) SET_SQUARE(X,Y,S)\
                                  corners[Y][X] = UPPERLEFT;\
                                  corners[Y][X + (S) - 1] = UPPERRIGHT;\
                                  corners[Y + (S) - 1][X] = LOWERLEFT;\
                                  corners[Y + (S) - 1][X + (S) - 1] = LOWERRIGHT;

#define UNSET_SQUARE_CORNERS(X,Y,S) UNSET_SQUARE(X,Y,S)\
                                  corners[Y][X] = 0;\
                                  corners[Y][X + (S) - 1] = 0;\
                                  corners[Y + (S) - 1][X] = 0;\
                                  corners[Y + (S) - 1][X + (S) - 1] = 0;

#define LIES_IN_SQUARE(SX,SY,S,X,Y) (((X) >= (SX)) && ((Y) >= (SY)) && ((X) < (SX + S)) && ((Y) < (SY + S)))

#define LIES_IN_MAIN_AREA(X,Y) (((X) < areaWidth) && ((Y) < areaHeight))

//symmetries of a square
//rotate 90 degrees
#define SYMM1_X(X,Y) (Y)
#define SYMM1_Y(X,Y) (areaWidth - (X) - 1)

//rotate 180 degrees
#define SYMM2_X(X,Y) (areaWidth - (X) - 1)
#define SYMM2_Y(X,Y) (areaHeight - (Y) - 1)

//rotate 270 degrees
#define SYMM3_X(X,Y) (areaWidth - (Y) - 1)
#define SYMM3_Y(X,Y) (X)

//reflect horizontally
#define SYMM4_X(X,Y) (areaWidth - (X) - 1)
#define SYMM4_Y(X,Y) (Y)

//reflect vertically
#define SYMM5_X(X,Y) (X)
#define SYMM5_Y(X,Y) (areaHeight - (Y) - 1)

//reflect diagonally 1
#define SYMM6_X(X,Y) (Y)
#define SYMM6_Y(X,Y) (X)

//reflect diagonally 2
#define SYMM7_X(X,Y) (areaWidth - (Y) - 1)
#define SYMM7_Y(X,Y) (areaWidth - (X) - 1)

//=============================EXPORT======================================

void printRectangle(FILE *f){
    int i, j;
    for(i = 0; i < areaHeight; i++){
        for(j = 0; j < areaWidth; j++){
            fprintf(f, "%d", grid[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\nThis solution has %d tile%s.\n\n", currentTiles,
            currentTiles == 1 ? "" : "s");
}

void printTikzHeader(FILE *f){
    int i;
    fprintf(f, "\\begin{tikzpicture}[yscale=-1, ultra thick, draw=black");
    for(i = smallestTile; i <= largestTile; i++){
        if(i <= COLOUR_NAMES_COUNT){
            fprintf(f, ",\n                    s%d/.style={fill=%s}", i, colours[i]);
        } else {
            fprintf(f, ",\n                    s%d/.style={fill=black}", i);
        }
    }
    fprintf(f, "]\n\n");
}

void tikzImageRectangle(FILE *f){
    int i, j;
    
    printTikzHeader(f);
    
    fprintf(f, "\\filldraw[s%d] (0,0) -- (%d,0) -- (%d,%d) -- (0,%d) -- (0,0);\n",
            GRID_VALUE(0,0), GRID_VALUE(0,0), GRID_VALUE(0,0), GRID_VALUE(0,0), GRID_VALUE(0,0));
    fprintf(f, "\\node at (%f, %f) {%d};\n", GRID_VALUE(0,0)/2.0, GRID_VALUE(0,0)/2.0, GRID_VALUE(0,0));
    
    for(i = 1; i < areaWidth; i++){
        if(GRID_VALUE(i,0) != GRID_VALUE(i-1,0)){
            fprintf(f, "\\filldraw[s%d] (%d,0) -- (%d,0) -- (%d,%d) -- (%d,%d) -- (%d,0);\n",
            GRID_VALUE(i,0), i, i + GRID_VALUE(i,0), i + GRID_VALUE(i,0), GRID_VALUE(i,0), i, GRID_VALUE(i,0), i);
            fprintf(f, "\\node at (%f, %f) {%d};\n", i + GRID_VALUE(i,0)/2.0, GRID_VALUE(i,0)/2.0, GRID_VALUE(i,0));
        }
    }
    
    for(i = 1; i < areaHeight; i++){
        if(GRID_VALUE(0,i) != GRID_VALUE(0, i-1)){
            fprintf(f, "\\filldraw[s%d] (0,%d) -- (%d,%d) -- (%d,%d) -- (0,%d) -- (0,%d);\n",
            GRID_VALUE(0,i), i, GRID_VALUE(0,i), i, GRID_VALUE(0,i), i + GRID_VALUE(0,i), i + GRID_VALUE(0, i), i);
            fprintf(f, "\\node at (%f, %f) {%d};\n", GRID_VALUE(0,i)/2.0, i + GRID_VALUE(0,i)/2.0, GRID_VALUE(0,i));
        }
        for(j = 1; j < areaWidth; j++){
            if((GRID_VALUE(j,i) != GRID_VALUE(j, i-1)) && (GRID_VALUE(j,i) != GRID_VALUE(j-1, i))){
                fprintf(f, "\\filldraw[s%d] (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d);\n",
                        GRID_VALUE(j,i),
                        j, i,
                        j + GRID_VALUE(j,i), i,
                        j + GRID_VALUE(j,i), i + GRID_VALUE(j, i),
                        j, i + GRID_VALUE(j, i),
                        j, i);
                fprintf(f, "\\node at (%f, %f) {%d};\n", 
                        j + GRID_VALUE(j,i)/2.0, i + GRID_VALUE(j,i)/2.0, GRID_VALUE(j,i));
            }
        }
    }
    
    fprintf(f, "\n\\end{tikzpicture}\n");
}

void tikzImageRectangleCorners(FILE *f){
    int i, j;
    
    printTikzHeader(f);
    
    for(i = 0; i < areaHeight; i++){
        for(j = 0; j < areaWidth; j++){
            if(IS_UPPERLEFT_CORNER(j, i) || GRID_VALUE(j,i)==1){
                fprintf(f, "\\filldraw[s%d] (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d) -- (%d,%d);\n",
                        GRID_VALUE(j,i),
                        j, i,
                        j + GRID_VALUE(j,i), i,
                        j + GRID_VALUE(j,i), i + GRID_VALUE(j, i),
                        j, i + GRID_VALUE(j, i),
                        j, i);
                fprintf(f, "\\node at (%f, %f) {%d};\n", 
                        j + GRID_VALUE(j,i)/2.0, i + GRID_VALUE(j,i)/2.0, GRID_VALUE(j,i));
            }
        }
    }
    
    fprintf(f, "\n\\end{tikzpicture}\n");
}

boolean checkCanonicity_symm1(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM1_X(x,y), SYMM1_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM1_X(x,y), SYMM1_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm2(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM2_X(x,y), SYMM2_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM2_X(x,y), SYMM2_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm3(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM3_X(x,y), SYMM3_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM3_X(x,y), SYMM3_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm4(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM4_X(x,y), SYMM4_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM4_X(x,y), SYMM4_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm5(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM5_X(x,y), SYMM5_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM5_X(x,y), SYMM5_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm6(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM6_X(x,y), SYMM6_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM6_X(x,y), SYMM6_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity_symm7(){
    int x, y;
    for(y = 0; y < areaHeight; y++){
        for(x = 0; x < areaWidth; x++){
            if(GRID_VALUE(x,y) < GRID_VALUE(SYMM7_X(x,y), SYMM7_Y(x,y))){
                return TRUE;
            } else if(GRID_VALUE(x,y) > GRID_VALUE(SYMM7_X(x,y), SYMM7_Y(x,y))){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean checkCanonicity(){
    if(areaWidth == areaHeight){
        //check all symmetries of a square
        return checkCanonicity_symm1() &&
                checkCanonicity_symm2() &&
                checkCanonicity_symm3() &&
                checkCanonicity_symm4() &&
                checkCanonicity_symm5() &&
                checkCanonicity_symm6() &&
                checkCanonicity_symm7();
    } else {
        //check all symmetries of a rectangle
        return checkCanonicity_symm2() &&
                checkCanonicity_symm4() &&
                checkCanonicity_symm5();
    }
}

void handleFinishedSquare(boolean noTouch){
    if(!skipCanonicityCheck && !checkCanonicity()){
        return;
    }
    solutionCount++;
    printRectangle(stderr);
    if(tikzOutput){
        if(noTouch){
            tikzImageRectangle(tikzOutputFile);
        } else {
            tikzImageRectangleCorners(tikzOutputFile);
        }
    }
}

void addNextNowhereNeatSquare(int lastX, int lastY){
    //first we look for the next free position
    int x, y;
    x = lastX;
    y = lastY;
    while(x < areaWidth && GRID_VALUE(x,y)){
        x++;
    }
    if(x == areaWidth){
        for(y = lastY + 1; y < areaHeight; y++){
            x = 0;
            while(x < areaWidth && GRID_VALUE(x,y)){
                x++;
            }
            if(x < areaWidth){
                break;
            }
        }
    }
    
    if(y > maxReachedHeight){
        maxReachedHeight = y;
    }
    
    if(x == areaWidth && y == areaHeight){
        //square is completely filled
        handleFinishedSquare(FALSE);
        return;
    }
    
    size minSize = smallestTile, maxSize = 0;
    
    while(maxSize < largestTile && LIES_IN_MAIN_AREA(x + maxSize, y + maxSize) &&
            (!GRID_VALUE(x + maxSize, y))){
        maxSize++;
    }
    
    size s;
    for(s = minSize; s <= maxSize; s++){
        if(y - 1 >= 0){
            if(GRID_VALUE(x, y - 1) == s && (s == 1 || IS_LOWERLEFT_CORNER(x, y-1))){
                continue;
            }
        }
        if(x - 1 >= 0){
            if(GRID_VALUE(x - 1, y) == s && (s == 1 || IS_UPPERRIGHT_CORNER(x - 1, y))){
                continue;
            }
        }
        SET_SQUARE_CORNERS(x, y, s);
        currentTiles++;
        addNextNowhereNeatSquare(x, y);
        if(currentTiles > maxTiles){
            maxTiles = currentTiles;
        }
        currentTiles--;
        UNSET_SQUARE_CORNERS(x, y, s);
    }
}

void addNextNoTouchSquare(int lastX, int lastY){
    //first we look for the next free position
    int x, y;
    x = lastX;
    y = lastY;
    while(x < areaWidth && GRID_VALUE(x,y)){
        x++;
    }
    if(x == areaWidth){
        for(y = lastY + 1; y < areaHeight; y++){
            x = 0;
            while(x < areaWidth && GRID_VALUE(x,y)){
                x++;
            }
            if(x < areaWidth){
                break;
            }
        }
    }
    
    if(y > maxReachedHeight){
        maxReachedHeight = y;
    }
    
    if(lastY == 0 && y > 0) {
        //when we finish the first row, we then
        //check that the first row is canonical
        int i = 0;
        while(i < areaWidth && GRID_VALUE(i, 0) == GRID_VALUE(SYMM4_X(i, 0), SYMM4_Y(i, 0))){
            i++;
        }
        if(i < areaWidth && GRID_VALUE(i, 0) > GRID_VALUE(SYMM4_X(i, 0), SYMM4_Y(i, 0))){
            //the first row was not canonical
            return;
        }
    }
    
    if(x == areaWidth && y == areaHeight){
        //square is completely filled
        handleFinishedSquare(TRUE);
        return;
    }
    
    if(areaWidth - x < 3 || areaHeight - y < 3){
        //no squares of size 1 or 2 in the outer 3 layers
        return;
    }
    
    size minSize = smallestTile, maxSize = 0;
    
    if(x < 3 || y < 3) {
        //no squares of size 1 or 2 in the outer 3 layers
        minSize = minSize < 3 ? 3 : minSize;
        
        if(x == 0 && y == 0){
            //no square of size 3 in the corners
            minSize = minSize < 4 ? 4 : minSize;
        }
    }
    
    while(maxSize < largestTile && LIES_IN_MAIN_AREA(x + maxSize, y + maxSize) &&
            (!GRID_VALUE(x + maxSize, y))){
        maxSize++;
    }
    
    size s;
    for(s = minSize; s <= maxSize; s++){
        if(y - 1 >= 0){
            int i = 0;
            while(i < s && GRID_VALUE(x + i, y - 1) != s) {
                i++;
            }
            if(i < s){
                continue;
            }
        }
        if(x - 1 >= 0){
            if(GRID_VALUE(x - 1, y) == s){
                continue;
            }
        }
        if(x + s + 1 < areaWidth){
            if(GRID_VALUE(x + s + 1, y) == s){
                continue;
            }
        }
        SET_SQUARE(x, y, s);
        currentTiles++;
        addNextNoTouchSquare(x, y);
        if(currentTiles > maxTiles){
            maxTiles = currentTiles;
        }
        currentTiles--;
        UNSET_SQUARE(x, y, s);
    }
}
//==============================USAGE======================================

void help(char *name) {
    fprintf(stderr, "The program %s constructs no-touch squarings of a given square.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options] n\n\n", name);
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -n, --neat\n");
    fprintf(stderr, "       Construct nowhere-neat tilings instead of no-touch tilings.\n");
    fprintf(stderr, "    -t, --tikz\n");
    fprintf(stderr, "       Write a tikzpicture for each solution to stdout.\n");
    fprintf(stderr, "    -s, --skip\n");
    fprintf(stderr, "       Skip canonicity check (some solutions might be counted several times).\n");
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
         {"min", required_argument, NULL, 'm'},
         {"max", required_argument, NULL, 'M'},
         {"neat", no_argument, NULL, 'n'},
         {"tikz", no_argument, NULL, 't'},
         {"skip", no_argument, NULL, 's'},
         {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;
    
    boolean nowhereNeat = FALSE;

    while ((c = getopt_long(argc, argv, "htsnm:M:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'm':
                smallestTile = (size)(atoi(optarg));
                break;
            case 'M':
                largestTile = (size)(atoi(optarg));
                break;
            case 'n':
                nowhereNeat = TRUE;
                break;
            case 't':
                tikzOutput = TRUE;
                break;
            case 's':
                skipCanonicityCheck = TRUE;
                break;
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
    
    if(smallestTile < 1){
        fprintf(stderr, "Tile sizes should be positive integers.\n");
        fprintf(stderr, "Setting minimum size to 1.\n");
        smallestTile = 1;
    }
    
    if(tikzOutputFile == NULL){
        // default to stdout
        tikzOutputFile = stdout;
    }
    
    if(argc - optind == 0 || argc - optind > 2){
        fprintf(stderr, "Illegal number of arguments.\n");
        usage(name);
        return EXIT_FAILURE;
    } else if (argc - optind == 1){
        areaWidth = areaHeight = (size)atoi(argv[optind]);
    } else {
        areaWidth = (size)atoi(argv[optind]);
        areaHeight = (size)atoi(argv[optind + 1]);
        if(areaHeight < areaWidth){
            size temp = areaHeight;
            areaHeight = areaWidth;
            areaWidth = temp;
        }
    }
    if(areaWidth > MAX_SIZE || areaHeight > MAX_SIZE){
        fprintf(stderr, "Squares of that size are not supported.\n");
        return EXIT_FAILURE;
    }
    
    if(nowhereNeat){
        addNextNowhereNeatSquare(0, 0);
    } else {
        addNextNoTouchSquare(0, 0);
    }
    
    fprintf(stderr, "Found %d solution%s.\n", 
            solutionCount, solutionCount == 1 ? "" : "s");
    if(solutionCount == 0){
        fprintf(stderr, "Maximum reached height: %d\n", maxReachedHeight);
    }
    fprintf(stderr, "Maximum number of tiles used: %d\n", maxTiles);
    
    return EXIT_SUCCESS;
}
