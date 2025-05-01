/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
     
    Modified and added to be Caleb Adesegun Samuel Adekoya
     Header for module which contains Prefix Trie 
        data structures and functions.
*/
struct prefixTree;

#ifndef PREFIXTREESTRUCT
#define PREFIXTREESTRUCT
#define CHILD_COUNT (1 << 8)
#define UNMARKED (0)
#define MARKED (1)
#define TRUE (1)
#define FALSE (0)

struct prefixTree;

/* A declaration of a node within a prefix tree */
struct prefixTree {

    char prefix; // The current prefix
    char *word;  // This value is only used for leaf nodes and stores
                 // a pointer to the word to avoid redudant computation
    int status;  // If a word is found here note that it's been found
    struct prefixTree **suffix; // Array of subtrees
    
    int **dMarked; // Used for part D only, Boolean matrix used 
                   // that trades memory for optimisation
};

#endif

/* Function that creates a new prefix tree node
    with contents intiliased to null and the memory allocated
    for the suffix subarray that contains pointers to the rest of 
    a potetial tree intialised as NULL */
struct prefixTree *newPrefixTree();

/* Inserts a new word into an existing prefix tree, by scaning through
    and creating new nodes where nessary */
struct prefixTree *addWordToTree(struct prefixTree *pt, char *word);

/* Frees the memory allocated for a given tree recursivley */
void freeTree(struct prefixTree *pt, int dim);
