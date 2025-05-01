/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Implementation for module which contains  
        Problem 1-related data structures and 
        functions.
    
    Sample solution implemented by Grady Fitzpatrick

    Modified and added to be Caleb Adesegun Samuel Adekoya
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "problem.h"
#include "problemStruct.c"
#include "solutionStruct.c"

/* Number of words to allocate space for initially. */
#define INITIALWORDSALLOCATION 64

/* Denotes that the dimension has not yet been set. */
#define DIMENSION_UNSET (-1)

#define LDINFINITY (LDBL_MAX / 2.0L)

// True and False boolean values
#define TRUE 1
#define FALSE 0

#define MATCH 1
#define DEL 2
#define INS 3
#define CONT 4

struct problem;
struct solution;

/* Sets up a solution for the given problem. */
struct solution *newSolution(struct problem *problem);

// Function Prototypes

// Reads the problem from input files
void readSequence(FILE *seqFile, int *seqLen, long double **seq);
// A constrianed version of DTW algorithm that sysmatically builds 
// minamised path lengths based on the k restraint
void contrainedWarp(struct problem *p, struct solution *s);
// Function that implements the dynamic DTW algorithm modifying the 
// solution's optimal value and optimal alignment matrix
void letsDoTheTW(struct problem *p, struct solution *s);
// Uses a given problems sequence dimensions and max path length
// to copute an (n+1) x (m+1) x (k) matrix intitilaised to LDINFINITY
long double ***bigMatrixMake(struct problem *p);
// Computes the absolute value of two sequences, at index i and j
long double absDist(struct problem *p, int i, int j);
// A function used to compare and compute the minumum of an [i,j] value's
// predacessors in a cost matrix returning the minumum
long double findmin(struct problem *p, struct solution *s, int i, int j);
// A boolean function that determines whether a given [i,j] is a valid
// member of a DTW cost matrix given the problems restrictions
int isValid(struct problem *p, int i, int j);


void readSequence(FILE *seqFile, int *seqLen, long double **seq){
    char *seqText = NULL;
    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&seqText, &allocated, '\0', seqFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading dictionary file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int seqTextLength = strlen(seqText);

    int commaCount = 0;
    /* Count how many numbers are present. */
    for(int i = 0; i < seqTextLength; i++){
        if(seqText[i] == ','){
            commaCount++;
        }
    }
    long double *seqLocal = (long double *) malloc(sizeof(long double) * (commaCount + 1));
    assert(seqLocal);

    int seqAdded = 0;
    while(progress < seqTextLength){
        int nextProgress;
        /* Read each value into the sequence. */
        assert(sscanf(seqText + progress, "%Lf , %n", &seqLocal[seqAdded], &nextProgress) == 1);
        assert(nextProgress > 0);
        progress += nextProgress;
        seqAdded++;
    }
    free(seqText);
    assert(seqAdded == (commaCount + 1));
    *seq = seqLocal;
    *seqLen = seqAdded;
}

/* 
    Reads the given dict file into a list of words 
    and the given board file into a nxn board.
*/
struct problem *readProblemA(FILE *seqAFile, FILE *seqBFile){
    struct problem *p = (struct problem *) malloc(sizeof(struct problem));
    assert(p);

    int seqALength = 0;
    long double *seqA = NULL;
    readSequence(seqAFile, &seqALength, &seqA);
    int seqBLength = 0;
    long double *seqB = NULL;
    readSequence(seqBFile, &seqBLength, &seqB);

    /* The length of the first sequence. */
    p->seqALength = seqALength;
    /* The first sequence. */
    p->sequenceA = seqA;

    /* The length of the second sequence. */
    p->seqBLength = seqBLength;
    /* The second sequence. */
    p->sequenceB = seqB;

    /* For Part D & F only. */
    p->windowSize = -1;
    p->maximumPathLength = -1;

    p->part = PART_A;

    return p;
}

struct problem *readProblemD(FILE *seqAFile, FILE *seqBFile, int windowSize){
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(seqAFile, seqBFile);

    p->part = PART_D;
    p->windowSize = windowSize;

    return p;
}

struct problem *readProblemF(FILE *seqAFile, FILE *seqBFile, 
    int maxPathLength){
    /* Interpretation of inputs is same as Part A. */
    struct problem *p = readProblemA(seqAFile, seqBFile);
    
    p->part = PART_F;
    p->maximumPathLength = maxPathLength;

    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution, 
    FILE *outfileName){
    assert(solution);
    fprintf(outfileName, "%.2Lf\n", solution->optimalValue);
    switch(problem->part){
        case PART_A:
            assert(solution->matrix);
            for(int i = 1; i <= problem->seqALength; i++){
                for(int j = 1; j <= problem->seqBLength; j++){
                    if(solution->matrix[i][j] == LDINFINITY){
                        fprintf(outfileName, "    ");
                    } else {
                        fprintf(outfileName, "%.2Lf", solution->matrix[i][j]);
                    }
                    if(j < (problem->seqBLength)){
                        /* Intercalate with spaces. */
                        fprintf(outfileName, " ");
                    }
                }
                fprintf(outfileName, "\n");
            }
            break;
        case PART_D:
        case PART_F:
            break;
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem){
    if(solution){
        if(solution->matrix){
            for(int i = 0; i < problem->seqALength + 1; i++){
                free(solution->matrix[i]);
            }
            free(solution->matrix);
        }
        
        if(solution->bigMatrix) {
            for (int i = 0; i < problem->seqALength + 1; i++) {
                if (solution->bigMatrix[i] != NULL) {
                    for (int j = 0; j < problem->seqBLength + 1; j++) {
                        free(solution->bigMatrix[i][j]);
                    }
            free(solution->bigMatrix[i]);
                }
            }
            free(solution->bigMatrix);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem){
    if(problem){
        if(problem->sequenceA){
            free(problem->sequenceA);
        }
        if(problem->sequenceB){
            free(problem->sequenceB);
        }
        free(problem);
    }
}

/* Sets up a solution for the given problem */
struct solution *newSolution(struct problem *problem){
    struct solution *s = (struct solution *) malloc(sizeof(struct solution));
    assert(s);
    if(problem->part == PART_F){
        s->matrix = NULL;
        s->bigMatrix = bigMatrixMake(problem);
    } else {
        s->bigMatrix = NULL;
        s->matrix = (long double **) malloc(sizeof(long double *) * 
            (problem->seqALength + 1));
        assert(s->matrix);
        for(int i = 0; i < (problem->seqALength + 1); i++){
            s->matrix[i] = (long double *) malloc(sizeof(long double) * 
                (problem->seqBLength + 1));
            assert(s->matrix[i]);
            for(int j = 0; j < (problem->seqBLength + 1); j++){
                s->matrix[i][j] = 0;
            }
        }
    }

    s->optimalValue = -1;
    
    return s;
}

/*See prototype above*/
int isValid(struct problem *p, int i, int j) {
    // Check whether they given i, j are within the bounds of
    // seqA and seqB resepectivley
    if (!(i<=p->seqALength && j<=p->seqBLength)) {
        return FALSE;
    }
    // If part D
    if (p->windowSize != -1) {
        // Check the given index i is within the window size valid at
        // the given height j
        if (!(((i <= (j + p->windowSize)) && (i >= (j - p->windowSize))))) {
            return FALSE;
        }
    }
    // All conditions hold the value is valid
    return TRUE;   
}

/*See prototype above*/
long double findmin(struct problem *p, struct solution* s, int i, int j) {
    // Intialise the minumum value to be the 'match' value at cost[i-1][j-1]
    // this safe for part A & D, given the base case of DTW
    long double min;
    min = s->matrix[i-1][j-1];

    // Check whether insertion at cost[i-1][j-1] minamises 
    if (isValid(p, i-1, j) && s->matrix[i-1][j]<min) {
        min = s->matrix[i-1][j];
    }
    // Check whether deletion at cost[i][j-1] minamises
    if (isValid(p, i, j-1) && s->matrix[i][j-1]<min) {
        min = s->matrix[i][j-1];
    }
    // Return computed min
    return min;
}

/*See prototype above*/
long double absDist(struct problem *p, int i, int j) {
    long double abs;
    // Compute the ordinary diffrence between the given points
    abs = p->sequenceA[i-1] - p->sequenceB[j-1];
    if (abs < 0) {
        // Abs < 0 make it positive
        abs = abs * -1;
    }
    // return abs
    return abs;
}  

/*See prototype above*/
long double ***bigMatrixMake(struct problem *p) {
    long double ***matrix;
    // Allocate the size space for a 3D Long Double array of size
    // (n+1) x (m+1) x k
    matrix = (long double ***) malloc(sizeof(long double **) * 
        (p->seqALength + 1));
    assert(matrix);

    for(int i = 0; i < (p->seqALength + 1); i++) {
        matrix[i] = (long double **) malloc(sizeof(long double *) * 
            (p->seqBLength + 1));
        assert(matrix[i]);

        for(int j = 0; j < (p->seqBLength + 1); j++) {
            matrix[i][j] = (long double *) malloc(sizeof(long double) * 
            (p->maximumPathLength));
            assert(matrix[i][j]);

            for(int k = 0; k < (p->maximumPathLength); k++) {
                // Intialise all values in the array to LDINFINITY
                matrix[i][j][k] = LDINFINITY;
            }           
               
        }
    }
    // returm the 3D matrix
    return matrix;
}

/*See prototype above*/
void letsDoTheTW(struct problem *p, struct solution *s) {
    // For Parts A & D initialise the solutions matrix to Infinity
    if (p->part != PART_F) {
        for (int i = 0; i < (p->seqALength + 1); i++) {
            for(int j = 0; j < (p->seqBLength + 1); j++){
                s->matrix[i][j] = LDINFINITY;
            }
        }
        // As per the DTW base case set the cost if matrix[0][0]
        // to 0 
        s->matrix[0][0] = 0;
    }

    // Populate the given cost matrix in acordance to the DTW
    // reccurance relation
    for (int i = 1; i < (p->seqALength + 1); i++) {

            for(int j = 1; j < (p->seqBLength + 1); j++){
                // Check whether i,j should be considred given the curr problem
                if (isValid(p, i, j)) {
                    // DTW reccurance 
                    s->matrix[i][j] = absDist(p, i, j) + findmin(p, s, i, j);
                }
            }
    } 
    // Update optimal value 
    s->optimalValue = s->matrix[p->seqALength][p->seqBLength];
}   

/*See prototype above*/
void contrainedWarp(struct problem *p, struct solution *s) {
    s->bigMatrix[1][1][0] = absDist(p, 1, 1);
    // initial path length of len 1
    long double min, abs;
    // each path length k compute the respective DTW matrix i,j. built off the 
    // path lengths contained in k - 1 
    for (int k = 1; k < p->maximumPathLength; k++) {
        // consider that for each path len k + 1, values of i and j 
        // should only be considred up until k + 1 while k < i/j
        for (int i = 1; i <= k+1 && i < (p->seqALength + 1); i++) {
            for(int j = 1; j <= k+1 && j < (p->seqBLength + 1); j++) {
                min = LDINFINITY;
                // If a minumum can not be found it indicates that
                // there exists no path lengths of k-1 for predacessors
                // and as such no path lengths of k exist for i,j
                if ((s->bigMatrix[i-1][j-1][k-1] != LDINFINITY) && 
                    (s->bigMatrix[i-1][j-1][k-1] < min)) {
                    // match
                    min = s->bigMatrix[i-1][j-1][k-1];
              
                }
                if ((s->bigMatrix[i-1][j][k-1] != LDINFINITY) &&
                    (s->bigMatrix[i-1][j][k-1] < min)) {
                    // insertion
                    min = s->bigMatrix[i-1][j][k-1];    
                    
                } 
                if ((s->bigMatrix[i][j-1][k-1] != LDINFINITY) &&
                    (s->bigMatrix[i][j-1][k-1] < min)) {
                    // deletion
                    min = s->bigMatrix[i][j-1][k-1];

                } 

                // If min == LDINFINITY then no valid value is 
                // found don't execute base case
                if (min != LDINFINITY) {
                    // DTW base case
                    abs = absDist(p, i, j) + min;
                    s->bigMatrix[i][j][k] = abs;
            
                } 
                
            }
        }   

    }
    long double mins = LDINFINITY;

    // For each optimalValue contained in the and k array matrix[n+1][m+1]
    // find the minimum
    for (int k=0; k < p->maximumPathLength; k++) {         
        if (s->bigMatrix[p->seqALength][p->seqBLength][k] < mins) {
            mins = s->bigMatrix[p->seqALength][p->seqBLength][k];
        }
    }
    if (mins != LDINFINITY) {  
        // if optimal value found edit it    
        s->optimalValue = mins;
    }
}
/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p){
    // alloc solution and perform a time warp
    struct solution *s = newSolution(p);
    letsDoTheTW(p, s);
    return s;
}
/*
    Solves the given problem according to Part D's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemD(struct problem *p){
    // alloc solution and perform a time warp based on window size
    struct solution *s = newSolution(p);
    letsDoTheTW(p, s);
    return s;
}
/*
    Solves the given problem according to Part F's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemF(struct problem *p){
    // alloc solution and perform a time warp based based on path len
    struct solution *s = newSolution(p);
    contrainedWarp(p, s);
    return s;
}

