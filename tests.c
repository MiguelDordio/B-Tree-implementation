#include "btree.h"
#include "queue.h"

int main(){

    element n1 = {.key = 20};
    element n2 = {.key = 30};
    element n3 = {.key = 10};
    element n4 = {.key = 40};
    element n5 = {.key = 15};
    element n6 = {.key = 17};
    element n7 = {.key = 18};
    element n8 = {.key = 50};
    element n9 = {.key = 60};
    element n10 = {.key = 70};

    FILE *fp;
    fp = fopen("file.bin", "wb+");

    bTree *tree = btCreate(4);

    btInsert(tree, n2, fp);
    btInsert(tree, n1, fp);
    btInsert(tree, n3, fp);
    btInsert(tree, n4, fp);
    btInsert(tree, n5, fp);
    btInsert(tree, n6, fp);
    btInsert(tree, n7, fp);
    btInsert(tree, n8, fp);
    btInsert(tree, n9, fp);
    btInsert(tree, n10, fp);


    queue *q = createQueue(15);

    btPrintTree(tree, q, fp);

    int pos = btSearch(tree->root, tree->order, n10, fp);
    if(pos != -1) {
        btNode x = disk_read(pos, tree->order, fp);
        printf("node has: %d keys", x.numKeys);
    }else
        printf("item doesnt exist!");

    return 0;
}