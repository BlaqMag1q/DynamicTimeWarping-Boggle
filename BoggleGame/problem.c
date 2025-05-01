/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Implementation for module which contains  
        Problem 2-related data structures and 
        functions.
    Modified and added to be Caleb Adesegun Samuel Adekoya
    
    Sample solution implemented by Grady Fitzpatrick
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "problem.h"
#include "problemStruct.c"
#include "solutionStruct.c"
#include "prefixTree.h"


/* Number of words to allocate space for initially. */
#define INITIALWORDSALLOCATION 64

/* Denotes that the dimension has not yet been set. */
#define DIMENSION_UNSET (-1)

#define NUMDIRECTIONS 8

struct problem;
struct solution;

/* Sets up a solution for the given problem. */
struct solution *newSolution(struct problem *problem);

/* Constructs a prefix tree based on a list words contained
    within the problem struct and the current problem */

struct prefixTree *buildTree (struct problem *p);

/* Utilises and adaptation of depth first search inorder
    to find words in a bobble graph, by congruently 
    searching the board and the prefix tree to */
void depthFirstFind(struct problem *p, struct prefixTree *pt, 
    struct solution *s, int **visited, int i, int j);

/* Determines whether a node should have 
    depthFirstFind performed on it */
int isIn(struct problem *p, struct prefixTree *pt, 
    int **visited, int i, int j);

/* A boolean function that returns TRUE or FALSE depending
    on if a function has no repeated chars or it does */
int noRepChars(char *word);

/* Establishes a dim x dim boolean matrix intiliased to be
    UNMARKED (0) */
int **visitMatrix(int dim);

/* Merge sort recursion function that splits arrays into subarrays */
void mergeSort(char **s, int l, int r);
/* Performs the merging component of merge sort sorting a given input array */
void mergeArrays(char **s, int l, int mid, int r);
/* Manipulates a char * into a char ** to re-use merge sort functions */
void charSort (struct solution *s);

/* 
    Reads the given dict file into a list of words 
    and the given board file into a nxn board.
*/
struct problem *readProblemA(FILE *dictFile, FILE *boardFile){
    struct problem *p = (struct problem *) malloc(sizeof(struct problem));
    assert(p);

    /* Part B onwards so set as empty. */
    p->partialString = NULL;

    int wordCount = 0;
    int wordAllocated = 0;
    char *dictText = NULL;
    char **words = NULL;

    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&dictText, &allocated, '\0', dictFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading dictionary file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    char *boardText = NULL;
    /* Reset allocated marker. */
    allocated = 0;
    success = getdelim(&boardText, &allocated, '\0', boardFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading board file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int dictTextLength = strlen(dictText);

    /* Count words present. */
    while(progress < dictTextLength){
        char *word = NULL;
        int nextProgress;
        /* Read each value into the dictionary. */
        if(progress == 0){
            /* First line. */
            int wordNumberGuess;
            assert(sscanf(dictText + progress, "%d %n", &wordNumberGuess, &nextProgress) == 1);
            /* Will fail if integer missing from the start of the words. */
            assert(nextProgress > 0);
            if(wordNumberGuess > 0){
                wordAllocated = wordNumberGuess;
                words = (char **) malloc(sizeof(char *) * wordAllocated);
                assert(words);
            }
            progress += nextProgress;
            continue;
        } else {
            assert(sscanf(dictText + progress, "%m[^\n] %n", &word, &nextProgress) == 1);
            assert(nextProgress > 0);
            progress += nextProgress;
        }

        /* Check if more space is needed to store the word. */
        if(wordAllocated <= 0){
            words = (char **) malloc(sizeof(char *) * INITIALWORDSALLOCATION);
            assert(words);
            wordAllocated = INITIALWORDSALLOCATION;
        } else if(wordCount >= wordAllocated){
            words = (char **) realloc(words, sizeof(char *) * 
                wordAllocated * 2);
            assert(words);
            wordAllocated = wordAllocated * 2;
            /* Something has gone wrong if there's not sufficient word 
                space for another word. */
            assert(wordAllocated > wordCount);
        }

        words[wordCount] = word;
        wordCount++;
    }
    
    /* Done with dictText */
    if(dictText){
        free(dictText);
    }
    
    /* Now read in board */
    progress = 0;
    int dimension = 0;
    int boardTextLength = strlen(boardText);
    /* Count dimension with first line */
    while(progress < boardTextLength){
        /* Count how many non-space characters appear in line. */
        if(boardText[progress] == '\n' || boardText[progress] == '\0'){
            /* Reached end of line. */
            break;
        }
        if(isalpha(boardText[progress])){
            dimension++;
        }
        progress++;
    }

    assert(dimension > 0);

    /* Check each line has the correct dimension. */
    for(int i = 1; i < dimension; i++){
        int rowDim = 0;
        if(boardText[progress] == '\n'){
            progress++;
        }
        while(progress < boardTextLength){
            /* Count how many non-space characters appear in line. */
            if(boardText[progress] == '\n' || boardText[progress] == '\0'){
                /* Reached end of line. */
                break;
            }
            if(isalpha(boardText[progress])){
                rowDim++;
            }
            progress++;
        }
        if(rowDim != dimension){
            fprintf(stderr, "Row #%d had %d letters, different to Row #1's %d letters.\n", i + 1, rowDim, dimension);
            assert(rowDim == dimension);
        }
    }

    /* Define board. */
    char *boardFlat = (char *) malloc(sizeof(char) * dimension * dimension);
    assert(boardFlat);
    
    /* Reset progress. */
    progress = 0;
    for(int i = 0; i < dimension; i++){
        for(int j = 0; j < dimension; j++){
            int nextProgress;
            assert(sscanf(boardText + progress, "%c %n", &boardFlat[i * dimension + j], &nextProgress) == 1);
            progress += nextProgress;
        }
    }

    char **board = (char **) malloc(sizeof(char **) * dimension);
    assert(board);
    for(int i = 0; i < dimension; i++){
        board[i] = &boardFlat[i * dimension];
    }

    // fprintf(stderr, "\n");

    /* The number of words in the text. */
    p->wordCount = wordCount;
    /* The list of words in the dictionary. */
    p->words = words;

    /* The dimension of the board (number of rows) */
    p->dimension = dimension;

    /* The board, represented both as a 1-D list and a 2-D list */
    p->boardFlat = boardFlat;
    p->board = board;

    /* For Part B only. */
    p->partialString = NULL;

    p->part = PART_A;
    free(boardText);
    return p;
}

struct problem *readProblemB(FILE *dictFile, FILE *boardFile, 
    FILE *partialStringFile){
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(dictFile, boardFile);

    char *partialString = NULL;

    /* Part B has a string that is partially given - we assume it follows 
        word conventions for the %s specifier. */
    assert(fscanf(partialStringFile, "%ms", &partialString) == 1);
    
    p->part = PART_B;
    p->partialString = partialString;

    return p;
}

struct problem *readProblemD(FILE *dictFile, FILE *boardFile){
    /* Interpretation of inputs is same as Part A. */
    struct problem *p = readProblemA(dictFile, boardFile);
    
    p->part = PART_D;
    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution, 
    FILE *outfileName){
    assert(solution);
    switch(problem->part){
        case PART_A:
        case PART_D:
            assert(solution->foundWordCount == 0 || solution->words);
            for(int i = 0; i < solution->foundWordCount; i++){
                fprintf(outfileName, "%s\n", solution->words[i]);
            }
            break;
        case PART_B:
            assert(solution->foundLetterCount == 0 || solution->followLetters);
            for(int i = 0; i < solution->foundLetterCount; i++){
                if(isalpha(solution->followLetters[i])){
                    fprintf(outfileName, "%c\n", solution->followLetters[i]);
                } else {
                    fprintf(outfileName, " \n");
                }
            }
            break;
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem){
    if(solution){
        if(solution->followLetters){
            free(solution->followLetters);
        }
        if(solution->words){
            free(solution->words);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem){
    if(problem){
        if(problem->words){
            for(int i = 0; i < problem->wordCount; i++){
                if(problem->words[i]){
                    free(problem->words[i]);
                }
            }
            free(problem->words);
        }
        if(problem->board){
            free(problem->board);
        }
        if(problem->boardFlat){
            free(problem->boardFlat);
        }
        if(problem->partialString){
            free(problem->partialString);
        }
        free(problem);
    }
}

/* Sets up a solution for the given problem */
struct solution *newSolution(struct problem *problem){
    struct solution *s = (struct solution *) malloc(sizeof(struct solution));
    assert(s);
    s->foundWordCount = 0;
    s->words = NULL;
    s->foundLetterCount = 0;
    s->followLetters = NULL;
    
    return s;
}

// See function prototype
int **visitMatrix(int dim) {
    // Alloctate the space for a 2D Int array
    int **visit;
    visit = (int **) malloc(sizeof(int *) * dim);
    assert(visit);

    for (int i = 0; i < dim; i++) {
        visit[i] = (int *) malloc(sizeof(int) * dim);
        assert(visit[i]);
        for (int j = 0; j < dim; j++) {
            // Ininialise the value to UNMARKED
            visit[i][j] = UNMARKED;
        }
    }
    // return the created matrix
    return visit;
}

/* JUSTIFICATION, for part d rather than cumulativley creating 
    a partician for each substring precomputation can be done 
    to avoid repeated chars altogether */
int noRepChars(char *word) {
    int *markedChar; 
    int len = strlen(word);  
    // Initialise a boolean array of size corresponding
    // to all possible characters  
    markedChar = (int *) malloc(sizeof(int) * CHILD_COUNT);
    assert(markedChar);
    for (int i = 0; i < CHILD_COUNT; i++){
        // Set all boolean values to UNMARKED
        markedChar[i] = UNMARKED;
    }
    // For each char in word mark unique chars until 
    // all words' chars are marked or a repeated char is found
    for (int i = 0; i < len; i++) {
        if (markedChar[(int)word[i]] == MARKED) {
            return MARKED;
        } else {
            markedChar[(int)word[i]] = MARKED;
        }
    } 
    free(markedChar);
    return UNMARKED;
}

/* As described in prototype */
int isIn(struct problem *p, struct prefixTree *pt, 
    int **visited, int i, int j) {

    // Check that i, j in the board, i, j is unvisited in the depth first,
    // and that there is a valid suffix tree for the index letter in the board     
    if (((i >= 0 && i < p->dimension) && (j >= 0 && j < p->dimension)) &&
     (visited[i][j] != MARKED) && (pt->suffix[tolower(p->board[i][j])] != NULL)) {
        if (p->part == PART_D) {
            // If it is part D check that the i, j is unseen following that specfic 
            // prefix in pt, if it is seen no need to check again 
            if (pt->suffix[tolower(p->board[i][j])]->dMarked == NULL) {
                return TRUE;
            } else if (pt->suffix[
                tolower(p->board[i][j])]->dMarked[i][j] == (MARKED)) {
                
                return FALSE;
            }              
        }
        return TRUE;    
    }
    // Invalid
    return FALSE;
}

/* As per prototype */
void depthFirstFind(struct problem *p, struct prefixTree *pt, struct
    solution *s, int **visited, int i, int j) {
    // Two sets that describe the series of possible connections 
    // of a given node i, j
    int diri[NUMDIRECTIONS] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dirj[NUMDIRECTIONS] = {-1, -1, -1, 0, 0, 1, 1, 1};

    // Check whether the current prefix is a word
    if (pt->suffix['\0'] != NULL) {
        if (pt->suffix['\0']->status == (UNMARKED)) {
            // Mark the word and add it
            s->words[s->foundWordCount] = pt->suffix['\0']->word;
            s->foundWordCount = s->foundWordCount + 1;  
            pt->suffix['\0']->status = (MARKED);  
        }
    }
    
    // The current node is visited for the search and for part D,
    // the current char being inspected in the prefix tree
    // is visited given the propety that it cant be visited again 
    // for the previous prefix again, alternativley using property
    // of no repeat it could also be updated to have it's predacessors
    // values as well. Not nessary given D though
    visited[i][j] = (MARKED);
    if (p->part == PART_D) {
        if (pt->dMarked == NULL) {
            pt->dMarked = visitMatrix(p->dimension);
        } 
        pt->dMarked[i][j] = (MARKED);

    } 
    // Perform a word search for all possible connetions if valid
    for (int k = 0; k < NUMDIRECTIONS; k++) {
        if (isIn(p, pt, visited, i + diri[k], j + dirj[k])) {
            depthFirstFind(p, pt->suffix[tolower(
                p->board[i + diri[k]][j + dirj[k]])], s,  
                visited, i + diri[k], j + dirj[k]);
        }
    }
    // Unvisit only the DFS visited array
    visited[i][j] = (UNMARKED);
}

/* As given in prototype */
struct prefixTree *buildTree (struct problem *p) {
    // Create a dummy node
    struct prefixTree *pt;
    pt = newPrefixTree();
    
    // Insert all words into the tree
    for (int i = 0; i < p->wordCount; i++) {
        if (p->part == PART_D && noRepChars(p->words[i]) != MARKED) {
            // If part D don't insert words with repeated chars
            addWordToTree(pt, p->words[i]);
        } else if (p->part != PART_D) {
            addWordToTree(pt, p->words[i]);
        }
    } 
    return pt;      
}

/* As per prototype */
void mergeSort(char **s, int l, int r) {
    // If the array size is >= 1
    if (l < r) {
        // Compute the midpoint
        int mid = l + (r - l) / 2;
        
        // Pass the left and right arrays
        mergeSort(s, l, mid);
        mergeSort(s, mid + 1, r);
        // Merge the arrays together to sort
        mergeArrays(s, l, mid, r);

    }
}

/* As per prototype */
void mergeArrays(char** s, int l, int mid,
    int r) {
    // Compute the left and right Array lengths
    int leftLen = mid - l + 1;
    int rightLen = r - mid;

    // Alocate size for the temporary Arrays
    char **tempL = ((char **) 
        malloc(sizeof(char*) * leftLen + 1));
    char **tempR = ((char **) 
        malloc(sizeof(char*) * rightLen + 1)); 
    assert(tempL && tempR);

    // Intialise counter variables
    int i, j, k;
    // Copy left and right arrays in to temp arrays
    for (i = 0; i < leftLen; i++) {
        tempL[i] = s[l + i]; 
    }
    for (i = 0; i < rightLen; i++) {
        tempR[i] = s[mid + 1 + i];  
    }
    // Now sort the aray comparing each word
    for (i=0, j=0, k=l; k<=r; k++) {

        if ((i<leftLen) && ((j>=rightLen) || 
            (strcmp(tempL[i], tempR[j]) < 0))) {
            s[k]=tempL[i];
            i++;
            
            
        } else {
            // Else an element from the right array should be added
            s[k]=tempR[j];
            j++; 
        }
    }
    free(tempL);
    free(tempR);
}

/* free visited array */
void freeVisit(int **visit, int dim) {
    for (int i = 0; i < dim; i++) {
        free(visit[i]);
    }    
    free(visit);
}

/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p) {
    struct solution *s = newSolution(p);
    int **visited = visitMatrix(p->dimension);
    struct prefixTree *pt = buildTree(p);
    
    // Allocate the max possible space required to avoid realloc
    char **buffer = (char **) malloc(sizeof(char*) * p->wordCount);
    assert(buffer);
    s->words = buffer;
    // Compute a depth first find for each i,j within the board
    for (int i = 0; i < p->dimension; i++) {
        for (int j = 0; j < p->dimension; j++) {
            if(isIn(p, pt, visited, i, j)) {
                // Valid node search
                depthFirstFind(p, pt->suffix[tolower(
                p->board[i][j])], s, visited, i, j);
            }
        }          
    }
    freeVisit(visited, p->dimension); 
    // Sort the answers
    mergeSort(s->words, 0, s->foundWordCount - 1);
    // Free the tree note second argument is because 
    // It is not PD
    freeTree(pt, 0);
    return s;
}
 /* As described in prototype */
void charSort (struct solution *s) {
    char **tempArray = (char **) malloc(sizeof(char*) * s->foundLetterCount);
    assert(tempArray);
    for (int i =0; i < s->foundLetterCount; i++) {
        // Turn a character into a string
        tempArray[i] = (char *) malloc(sizeof(char)+1);
        assert(tempArray[i]); 
        tempArray[i][0] = s->followLetters[i];
        // Add a terminating character
        tempArray[i][1] = '\0';
    }
    // Sort using the char** merge sort
    mergeSort(tempArray, 0, s->foundLetterCount - 1);
    // Re copy the array back into char *
    for (int i =0; i < s->foundLetterCount; i++) {
        s->followLetters[i] = tempArray[i][0];
        free(tempArray[i]);
    }
    free(tempArray);
}
/*
    Solves the given problem according to Part B's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemB(struct problem *p) {
    struct solution *s = newSolution(p);
    int **visited = visitMatrix(p->dimension);
    struct prefixTree *pt = buildTree(p);
    int partLen = strlen(p->partialString);

    if (partLen <= 0) {
        // if the partial string length is <= it is not a valid string
        return s;
    }
    // Can't find more found words than words in dict
    char **buffer = (char **) malloc(sizeof(char*) * p->wordCount);
    assert(buffer);
    s->words = buffer;
    for (int i = 0; i < p->dimension; i++) {
        for (int j = 0; j < p->dimension; j++) {
            if(isIn(p, pt, visited, i, j)) {
                // Only perform depth first find of positions in the board that
                // correlate to that first char of partial string
                if (tolower(p->board[i][j]) == tolower(p->partialString[0])){
                    depthFirstFind(p, pt->suffix[tolower(
                    p->board[i][j])], s, visited, i, j);
                }
                
            }
        }          
    }
    // Establish a seen array to avoid redudantly printing the same char
    int *seen = (int *) malloc(sizeof(int) * CHILD_COUNT);
    for(int i = 0; i < CHILD_COUNT; i++) {
        seen[i] = FALSE;
    }
    // Can't be more follow chars than words found
    char *followLetters = (char *) malloc(sizeof(char) * s->foundWordCount);
    assert(followLetters);
    s->followLetters = followLetters;
    for (int i = 0; i < s->foundWordCount; i++) {
        // If found word is < partial string it cant contain it
        if (strlen(s->words[i]) >= partLen) {
            // String match marker
            int match = TRUE;
            for (int k = partLen-1; k >= 0; k--) {
                if (s->words[i][k] != tolower(p->partialString[k])) {
                    // If chars don't match partial strings don't match
                    match = FALSE;
                }
            }
            if (match == TRUE && seen[(int)s->words[i][partLen]] == FALSE) {
                // It is a match and hasn't been seen, add it to follow chars
                s->followLetters[s->foundLetterCount] = s->words[i][partLen];
                s->foundLetterCount = s->foundLetterCount + 1;     
                seen[(int)s->words[i][partLen]] = TRUE;  
            }
        }
    }

    free(seen);
    // Sort and return the output
    charSort(s);
    freeVisit(visited, p->dimension); 
    freeTree(pt, 0);
    return s;
}
/*
    Solves the given problem according to Part D's definition
    and places the solution output into a returned solution value.
    it exploits the idea that since chars can't be repeated, to say
    that any parallel search on the tree and board for a spefic prefix
    eg. x...y, with suffix eg z cant revisit any i,j = z that has 
    already been searched following that prefix, or any chars
    also visited by searching the prefix, this partician is created
    by using a 2D matrix in tree nodes, to record i,j's visited
    at that node, and by only inserting words with no repeated chars
*/
struct solution *solveProblemD(struct problem *p){
    struct solution *s = newSolution(p);
    int **visited = visitMatrix(p->dimension);
    struct prefixTree *pt = buildTree(p);
    // Can't find more words than dict size
    char **buffer = (char **) malloc(sizeof(char*) * p->wordCount);
    assert(buffer);
    s->words = buffer;
    // Perform Depth First Find for elements in the board
    // particioning indvidual searches as mentioned above
    for (int i = 0; i < p->dimension; i++) {
        for (int j = 0; j < p->dimension; j++) {
            if(isIn(p, pt, visited, i, j)) {
                depthFirstFind(p, pt->suffix[tolower(
                p->board[i][j])], s, visited, i, j);
            }
        }          
    }
    // Sort and return the found words
    mergeSort(s->words, 0, s->foundWordCount - 1);
    freeVisit(visited, p->dimension); 
    freeTree(pt, p->dimension);
    return s;
}

