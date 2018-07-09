#ifndef BTREE_H
# define BTREE_H

#include <stdio.h>
#include <malloc.h>
#include "queue.h"


//#############################################################################
//                               STRUCTS
//#############################################################################

typedef struct element{
    int key;              // the key of the element
    int data;             // that data that each element contains
}element;

typedef struct btNode{
    int numKeys;          // nº of keys the node has
    int isLeaf;           // is this a leaf node? 1 = true, 0 = false
    int pos_in_disk;      // position of the node in the file
    element *keys;        // holds the keys of the node
    int *kids;            // holds the children of the node
}btNode;

typedef struct bTree {
    int order;            // order of the B-Tree
    btNode root;          // root of the B-Tree
    int node_count;       // total nº of nodes the tree has
} bTree;

typedef struct queue queue;

//#############################################################################
//                               METHODS
//#############################################################################

// create a new empty tree
bTree *btCreate(int order);

// return nonzero if key is present in tree
int btSearch(btNode node, int order, element key, FILE *fp);

// insert a new element into a tree
void btInsert(bTree *tree, element key, FILE *fp);

// deletes a key from the tree
void btDelete(bTree *tree, element key, FILE *fp);

// print all keys of the tree from the root
void btPrintTree(bTree *tree, queue *q,FILE *fp);

// free the tree
void btDestroy(bTree *tree, FILE *fp);

// find the max element of the tree
element btfindMax(btNode node, int order, FILE *fp);

// find the min element of the tree
element btfindMin(btNode node, int order, FILE *fp);

btNode disk_read(int disk, int order, FILE *fp);

#endif