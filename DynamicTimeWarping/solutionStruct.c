
/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Implementation for data structure used
        in storing the solution and its
        information.

    You may change this file if you would
        like to add additional fields.
    Modified and added to be Caleb Adesegun Samuel Adekoya
    SID, 1461186
*/
struct solution {
    /* The required (n + 1) x (m + 1) matrix. Only for Part A and D. */
    long double **matrix;
    // An (n + 1) x (m + 1) * (k) matrix, where k is the path length
    long double ***bigMatrix;
    /* The final optimal value (bottom-right value). */
    long double optimalValue;
};
