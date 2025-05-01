/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Modified and added to by Caleb Adesegun Samuel Adekoya
     Implementation for module which contains Prefix Trie 
        data structures and functions.
*/
#include "prefixTree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

/* Function that creates a new prefix tree node
    with contents intiliased to null and the memory allocated
    for the suffix subarray that contains pointers to the rest of 
    a potetial tree intialised as NULL */
struct prefixTree *newPrefixTree() {
    struct prefixTree *retTree = NULL;
    struct prefixTree **suff = NULL;
    // Alloc the nessary space for a given node
    retTree = (struct prefixTree *) malloc(sizeof(struct prefixTree));
    assert(retTree);
    // intiliase as '\0' for dummy node
    retTree->prefix = '\0';
    retTree->word = NULL;
    retTree->status = UNMARKED;

    // Allocate the space for an array of subtrees of length child count
    suff = (struct prefixTree **) malloc(sizeof(struct prefixTree *) 
        * CHILD_COUNT);
    assert(suff);
    retTree->suffix = suff;
    for (int i = 0; i < CHILD_COUNT; i++) {
        // initialise subtrees to NULL
        retTree->suffix[i] =  NULL;
    }
    // Used for part D set as NULL
    retTree->dMarked = NULL;
    return retTree;
}

/* Inserts a new word into an existing prefix tree, by scaning through
    and creating new nodes were nessary */
struct prefixTree *addWordToTree(struct prefixTree *pt, char *word) {
    // If the word is empty it cant be added
    if (word[0] == '\0') {
        return pt;
    }   
    int len = strlen(word);
    int newChar = FALSE;    // Boolean flag for if a new suffix is found
    int i = 0;
    char tempChar;
    struct prefixTree *node = pt;

    // Interate through word and search the three until a new suffix is 
    // found or until the word in empty (i >=len)
    while (newChar != TRUE && i < len) {
        // The search is caps insensetive
        tempChar = tolower(word[i]);
        if (node->suffix[(int)tempChar] == NULL) {
            // no current suffix tree create new
            newChar = TRUE;
            continue;
        } else {
            i++;
            node = node->suffix[(int)tempChar];
        }
    }
        
    // Copy out the rest of the characters in the word creating new nodes
    // and inserting them
    while (i < len) {
        tempChar = tolower(word[i]);
        node->suffix[(int)tempChar] = newPrefixTree();
        node = node->suffix[(int)tempChar];
        node->prefix = tempChar;
        i++;
    }
    // Store the word and it's end node in the '/0' suffix
    node->suffix['\0'] = newPrefixTree();
    node->suffix['\0']->word = word;
    // Return the computed tree
    return pt;
}

/* Frees the memory allocated for a given tree recursivley */
void freeTree(struct prefixTree *pt, int dim) {    
    if (pt == NULL) {
        // Empty Tree
        return;
    }
    if (pt->dMarked) {
        // if the boolean matrix has been used free it
        for (int i = 0; i < dim; i++) {
            free(pt->dMarked[i]);
        }   
        free(pt->dMarked);
    }
    // Recursivley free each subsequent child node
    for (int i = 0; i < CHILD_COUNT; i++) {
        freeTree(pt->suffix[i], dim);    
    }
    // Free the suffix array and current node
    free(pt->suffix);
    free(pt);
}

