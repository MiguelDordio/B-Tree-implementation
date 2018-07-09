#include "btree.h"

//#############################################################################
//                               HELPER METHODS
//#############################################################################

int calculate_offset(int disk, int order){

    // calculate the nº of bytes a node has
    int size_of_btNode = (sizeof(int) * 3) + (sizeof(element) * order-1) + (sizeof(int) * order);

    return size_of_btNode * disk;    // calculate the position of the node in the file

}

btNode disk_read(int disk, int order, FILE *fp){
    btNode read_node;

    int offset = calculate_offset(disk, order);
    fseek(fp, offset, SEEK_SET);                                    // set the file pointer there


    fread(&read_node.numKeys, sizeof(read_node.numKeys), 1, fp);    // read the information from the file
    fread(&read_node.isLeaf, sizeof(read_node.isLeaf), 1, fp);
    fread(&read_node.pos_in_disk, sizeof(read_node.pos_in_disk), 1, fp);

    read_node.keys = malloc(sizeof(element) * order-1);
    fread(read_node.keys, sizeof(element), order-1, fp);

    read_node.kids = malloc(sizeof(int) * order);
    fread(read_node.kids, sizeof(int), order, fp);


    return read_node;
}

void disk_write(btNode node, int order, FILE *fp){

    int offset = calculate_offset(node.pos_in_disk, order);
    fseek(fp, offset, SEEK_SET);                                 // set the file pointer there

    fwrite(&node.numKeys, sizeof(node.numKeys), 1, fp);          // write the information to the file
    fwrite(&node.isLeaf, sizeof(node.isLeaf), 1, fp);
    fwrite(&node.pos_in_disk, sizeof(node.pos_in_disk), 1, fp);
    fwrite(node.keys, sizeof(element), order-1, fp);
    fwrite(node.kids, sizeof(int), order, fp);
}

btNode new_node(int order, int is_leaf) {
    btNode n;

    n.numKeys = 0;                                 // set nº of keys to 0
    n.isLeaf = is_leaf;

    n.keys = malloc((order-1) * sizeof(element));  // allocate space for the array of keys
    for(int i=0; i < order-1; i++){                // initialize the keys in the array
        n.keys[i].key = -1;
        n.keys[i].data = -1;
    }

    n.kids = malloc((order) * sizeof(int));        // allocate space for the array of keys
    for(int i=0; i < order; i++){                  // initialize the kids in the array
        n.kids[i] = -1;
    }

    return n;
}

void print_node_keys(btNode node, int order){
    printf("[");
    for(int i = 0; i < order-1; i++){
        if(node.keys[i].key != -1)
            printf("key: %d, ", node.keys[i].key);
    }
    printf("] ");
}

//#############################################################################
//                               INSERTION
//#############################################################################

void bt_split_child(btNode x, int pos, bTree *tree, FILE *fp, int split_root){

    btNode y = disk_read(x.kids[pos], tree->order, fp); // node to split (pos-th child)
    if(split_root == 1){                                // special case when splitting the root of the tree
        tree->node_count++;                             // increment nº of total nodes
        y.pos_in_disk = tree->node_count;               // attribute a new location in the file
    }
    btNode z = new_node(tree->order, y.isLeaf);         // new (pos+1)-th child
    tree->node_count++;                                 // increment nº of total nodes
    z.pos_in_disk = tree->node_count;                   // attribute a new location in the file
    int t = (tree->order / 2);                          // calculate minimum ramification degree

    if(tree->order % 2 == 0){
        t--;
    }
    z.numKeys = t;                                      // nº of keys the new node will receive

    if(tree->order % 2 != 0){
        t--;
    }
    for(int j = 0; j <= t && (j+t+1)<= y.numKeys-1; j++){ // move elements to new node
        z.keys[j] = y.keys[j+t+1];
        y.keys[j+t+1].key = -1;                         // erase the element from the previous node
        y.keys[j+t+1].data = -1;
    }

    if(y.isLeaf == 0){                                  // if y is not a leaf
        for(int j = 0; j <= t; j++){                    // move children as well
            z.kids[j] = y.kids[j+t+1];
            y.kids[j+t+1] = -1;                         // erase the element from the previous node
        }
    }
    y.numKeys = t;                                      // update the nº of keys the node has after split


    if(split_root == 1){                                // special case when splitting the root of the tree
        x.kids[pos] = y.pos_in_disk;
        x.kids[pos+1] = z.pos_in_disk;
    }else{
        int j, i, r;
        for(j = 0; j < tree->order;j++){                 // make room for x`s new child
            if(x.kids[j] == y.pos_in_disk){
                for(i = j+1; i < tree->order;i+=2){
                    if(i+1 < tree->order)
                        x.kids[i+1] = x.kids[i];
                }
                r = j;
            }
        }
        x.kids[r+1] = z.pos_in_disk;
    }


    for(int j = pos; j < tree->order-2; j+=2){           // make room for the element
        x.keys[j+1] = x.keys[j];                         // that will be promoted
    }

    x.keys[pos] = y.keys[t];                             // promote element
    y.keys[t].key = -1;                                  // erase the updated element from the previous node
    y.keys[t].data = -1;
    x.numKeys++;                                         // increment the nº of keys the root node has

    disk_write(x, tree->order, fp);                      // update the information in the file
    disk_write(y, tree->order, fp);                      // update the information in the file
    disk_write(z, tree->order, fp);                      // update the information in the file
}

btNode bt_insert_nonfull(btNode node, element key, bTree *tree, FILE *fp){

    int pos = node.numKeys;

    if(node.isLeaf == 1){                                      // if in a leaf insert the new element
        int i = pos-1;
        while(i >= 0 && key.key < node.keys[i].key){           // find the correct position
            node.keys[i+1] = node.keys[i];
            node.keys[i].key = -1;
            node.keys[i].data = -1;
            i--;
        }
        if(i+1 != pos){
            node.keys[i+1] = key;
        }else{
            node.keys[pos] = key;
        }
        node.numKeys++;
        disk_write(node, tree->order, fp);
        return node;
    }else{                                                     // otherwise, descend to the appropriate child
        int n_pd = node.pos_in_disk;
        int i = pos-1;
        while (key.key < node.keys[i].key && i >= 0) {         // get the correct child of the node
            i--;
            pos--;
        }

        btNode x = disk_read(node.kids[pos], tree->order, fp); // get the child node
        if(x.numKeys == tree->order-1){                        // is this child full?
            bt_split_child(node, pos, tree, fp, 0);            // split the child
            btNode x1 = disk_read(n_pd, tree->order, fp);      // get the updated node
            if(key.key > x1.keys[pos].key)                     // adjust the position if needed
                pos++;
        }
        btNode x1 = disk_read(n_pd, tree->order, fp);          // get the updated node
        btNode x2 = disk_read(x1.kids[pos], tree->order, fp);  // get the child node
        bt_insert_nonfull(x2, key, tree, fp);
    }
}

//#############################################################################
//                               DELETION
//#############################################################################

element bt_delete_max(btNode node, int order, FILE *fp){
    if(node.isLeaf == 1) {
        node.keys[node.numKeys-1].key = -1;
        node.keys[node.numKeys-1].data = -1;
        node.numKeys--;
        disk_write(node, order, fp);
        return node.keys[node.numKeys-1];
    }else{
        btNode x = disk_read(node.kids[node.numKeys], order, fp);
        bt_delete_max(x, order, fp);
    }
}

element bt_delete_min(btNode node, int order, FILE *fp){
    if(node.isLeaf == 1) {
        element x = node.keys[0];
        for(int j = 0; j < node.numKeys; j++)
            node.keys[j] = node.keys[j+1];
        node.numKeys--;
        disk_write(node, order, fp);
        return x;
    }else{

        btNode x = disk_read(node.kids[0], order, fp);
        bt_delete_min(x, order, fp);
    }
}

void bt_merge_children(btNode node, int pos, int order, FILE *fp){
    int t = (order / 2);

    btNode y = disk_read(node.kids[pos], order, fp);   // merge children pos
    btNode z = disk_read(node.kids[pos+1], order, fp); // and pos+1

    y.keys[t-1] = node.keys[pos];                      // borrow item from the root
    node.keys[pos].key = -1;                           // delete item the item borrowed
    node.keys[pos].data = -1;
    for(int j = 0; j < t-1;j++){                       // transfer kids[pos+1]
        y.keys[t + j] = z.keys[j];                     // contents to kids[pos]
    }
    if(y.isLeaf == 0){
        for(int j = 0; j < t;j++){
            y.kids[t + j] = z.kids[j];
        }
    }
    y.numKeys = order -1;                              // kids[pos] is now full
    for(int j = pos+1; j < node.numKeys;j++){
        node.keys[j-1] = node.keys[j];
    }
    for(int j = pos+2; j < node.numKeys+1;j++){
        node.kids[j-1] = node.kids[j];
    }
    node.numKeys--;
    //free(z);                                         // delete old kids[pos+1]
    disk_write(y, order, fp);
    disk_write(node, order, fp);
}

void bt_borrow_from_left_sibling(btNode node, int pos, int order, FILE *fp){
    int t = (order / 2);
    btNode y = disk_read(node.kids[pos], order, fp);    // node pos`s left
    btNode z = disk_read(node.kids[pos-1], order, fp);  // sibling is pos-1

    for(int j = t-1; j > 0; j--){                       // make room for
        y.keys[j] = y.keys[j-1];                        // new 1st key
    }

    y.keys[0] = node.keys[pos-1];                       // item from the root comes down
    node.keys[pos-1] = z.keys[z.numKeys-1];             // borrow the last item from the left node

    z.keys[z.numKeys-1].key = -1;                       // remove borrowed item
    z.keys[z.numKeys-1].data = -1;

    if(y.isLeaf == 0){
        for(int j = t; j > 1; j--){                     // make room for
            y.kids[j+1] = y.kids[j];                    // new 1st child
        }
        y.kids[1] = z.kids[z.numKeys+1];
    }
    y.numKeys = t;
    z.numKeys--;
    disk_write(z, order, fp);
    disk_write(y, order, fp);
    disk_write(node, order, fp);
}

void bt_borrow_from_right_sibling(btNode node, int pos, int order, FILE *fp){
    int t = (order / 2);
    btNode y = disk_read(node.kids[pos], order, fp);    // node pos`s left
    btNode z = disk_read(node.kids[pos+1], order, fp);  // sibling is pos+1

    y.keys[y.numKeys] = node.keys[pos];                 // item from the root comes down
    node.keys[pos] = z.keys[0];                         // borrow the first item from the right node

    for(int j = 0; j < z.numKeys;j++){                  // adjust the keys after of the right node
        if(j+1 == z.numKeys){
            z.keys[j].key = -1;
            z.keys[j].data = -1;
        }else
            z.keys[j] = z.keys[j+1];
    }


    if(y.isLeaf == 0){
        for(int j = t; j > 1; j--){                     // make room for
            y.kids[j+1] = y.kids[j];                    // new last child
        }
        y.kids[1] = z.kids[z.numKeys+1];
    }
    y.numKeys = t;
    z.numKeys--;
    disk_write(z, order, fp);
    disk_write(y, order, fp);
    disk_write(node, order, fp);
}

void bt_delete_safe(btNode node, element key, int order, FILE *fp){
    int t = (order / 2);
    int borrowed; //default
    int pos = 0;
    while(pos <= node.numKeys-1 && key.key > node.keys[pos].key)
        pos++;
    if(pos <= node.numKeys && key.key == node.keys[pos].key){
        if(node.isLeaf == 1){                                         // case 1
            for(int j = pos; j < node.numKeys; j++)                   // case 1
                node.keys[j] = node.keys[j+1];                        // case 1
            if(pos == node.numKeys-1){
                node.keys[pos].key = -1;
                node.keys[pos].data = -1;
            }
            node.numKeys--;                                           // case 1
            disk_write(node, order, fp);                              // case 1
        }else{
            btNode y = disk_read(node.kids[pos], order, fp);
            if(y.numKeys > t-1){                                      // case 2a
                node.keys[pos] = bt_delete_max(y, order, fp);         // case 2a
                disk_write(node, order, fp);                          // case 2a
            }else{
                btNode z = disk_read(node.kids[pos+1], order, fp);
                if(z.numKeys > t-1){                                  // case 2b
                    node.keys[pos] = bt_delete_min(z, order, fp);     // case 2b
                    disk_write(node, order, fp);                      // case 2b
                }else{
                    bt_merge_children(node, pos, order, fp);          // case 2c
                    btNode node_child = disk_read(node.kids[pos], order, fp);
                    bt_delete_safe(node_child, key, order, fp);       // case 2c
                }
            }
        }
    }else if(node.isLeaf == 0){
        int m = pos; //default
        btNode y = disk_read(node.kids[pos], order, fp);
        if(y.numKeys == t-1){
            borrowed = 0;
            if(pos > 0){
                btNode z = disk_read(node.kids[pos-1], order, fp);
                if(z.numKeys > t - 1){                                 // case 3a
                    bt_borrow_from_left_sibling(node, pos, order, fp); // case 3a
                    borrowed = 1;                                      // case 3a
                }else{
                    m = pos - 1;
                }
            }
            if(borrowed == 0 && pos <= node.numKeys && node.kids[pos+1] != -1){
                btNode z = disk_read(node.kids[pos+1], order, fp);
                if(z.numKeys > t - 1){                                 // case 3a
                    bt_borrow_from_right_sibling(node, pos, order, fp);// case 3a
                    borrowed = 1;                                      // case 3a
                }else{
                    m = pos;
                }
            }
            if(borrowed == 0){                                         // case 3b
                bt_merge_children(node, m, order, fp);                 // case 3b
                btNode x = disk_read(node.kids[m], order, fp);
                y = x;                                                 // case 3b
            }
        }
        if(m != pos){
            bt_delete_safe(y, key, order, fp);
        }else{
            btNode new_y = disk_read(node.kids[pos], order, fp);
            bt_delete_safe(new_y, key, order, fp);
        }
    }
}

//#############################################################################
//                               METHODS
//#############################################################################


bTree *btCreate(int order){

    bTree *tree;                                // creates the "header" of the B-Tree
    if((tree = malloc(sizeof(bTree))) == NULL)  // allocate space for the new tree
        return NULL;

    btNode root = new_node(order, true);        // creates the root of the new B-Tree
    root.pos_in_disk = 0;                       // give the root a position in the file

    tree->order = order;                        // give the tree it`s order
    tree->root = root;                          // give the tree it`s root
    tree->node_count = 0;                       // set the tree`s node count to 0

    return tree;

}

void btInsert(bTree *tree, element key, FILE *fp){
    if(tree->node_count > 0)
        tree->root = disk_read(0, tree->order, fp);           // update the root of the tree
    btNode root = tree->root;

    if(root.numKeys == tree->order-1){                        // if the root is full
        btNode s = new_node(tree->order, 0);                  // create a new root node
        s.kids[0] = root.pos_in_disk;                         // root becomes the first child
        bt_split_child(s, 0, tree, fp, 1);                    // split the root
        s = disk_read(0, tree->order, fp);                    // get the new root
        tree->root = s;                                       // make it the new root after the split
        bt_insert_nonfull(s, key, tree, fp);                  // now insert the new element
    }else{
        tree->root = bt_insert_nonfull(root, key, tree, fp);  // insert the new element in a non-full node
    }

}

int btSearch(btNode node, int order, element key, FILE *fp){

    int pos = 0;
    while(pos < node.numKeys && key.key > node.keys[pos].key){  // find the correct position
        pos++;
    }
    if(pos <= node.numKeys && key.key == node.keys[pos].key){   // is the item one of the key`s of this node?
        return node.pos_in_disk;
    }else if(node.isLeaf == 1){                                 // if a leaf was hit and no item was found
        return -1;
    }else{
        btNode x = disk_read(node.kids[pos], order, fp);        // go deeper in the tree
        return btSearch(x, order, key, fp);
    }
}

void btDelete(bTree *tree, element key, FILE *fp){

    btNode root = tree->root;
    bt_delete_safe(root, key, tree->order, fp);                  // delete the item
    btNode new_root = disk_read(0, tree->order, fp);
    if(new_root.numKeys == 0 && (new_root.isLeaf == 0)){         // is the root now empty and not a leaf?
        btNode x = disk_read(new_root.kids[0], tree->order, fp); // get the first child of the root
        x.pos_in_disk = 0;                                       // overwrite the previous root info
        disk_write(x, tree->order, fp);                          // in the file with the child info
        tree->root = x;                                          // make the child be the root
    }else{
        tree->root = new_root;
    }
}

element btfindMax(btNode node, int order, FILE *fp){
    if(node.isLeaf == 1) {
        return node.keys[node.numKeys-1];
    }else{
        btNode x = disk_read(node.kids[node.numKeys], order, fp);
        btfindMax(x, order, fp);
    }
}

element btfindMin(btNode node, int order, FILE *fp){
    if(node.isLeaf == 1) {
        return node.keys[0];
    }else{

        btNode x = disk_read(node.kids[0], order, fp);
        btfindMin(x, order, fp);
    }
}

void btPrintTree(bTree *tree, queue *q,FILE *fp){
    if(tree->root.numKeys == 0){
        printf("\nThe B-Tree is empty\n");
    }else{
        btNode end = { .numKeys = -1};                    // marker to know when a level of the tree ends
        insert(q, tree->root);                            // insert the root in the queue
        int item_count= 1;                                // real item/node counter
        while(!isEmpty(q)){
            btNode current = removeData(q);               // remove the first item in the queue and return that node
            if(current.numKeys == -1){                    // was a marker found?
                printf("\n");
                insert(q, end);
                if(item_count == 0)                       // to avoid and endless loop of markers
                    break;                                // when the tree is already printed
            }else{
                item_count--;
                print_node_keys(current, tree->order);
                if(current.pos_in_disk == 0)              // special case for the root
                    insert(q, end);
                for(int i = 0; i < tree->order; i++){     // insert all the kids os the next node in the queue
                    if(current.kids[i] != -1){
                        btNode x = disk_read(current.kids[i], tree->order, fp); // get the kid
                        insert(q, x);
                        item_count++;
                    }
                }
            }
        }
    }
}

void btDestroy(bTree *tree, FILE *fp){
    free(tree);
    fclose(fp);
    if(remove("file.bin") == 0)
        printf("\nFile deleted successfully\n");
    else
        printf("\nError: unable to delete the file\n");
}
