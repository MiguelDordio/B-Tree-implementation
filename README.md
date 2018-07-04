# B-Tree implementation
B-Tree Implementation in secundary-memory/disk-memory

I will start by saying this implementation was based on the approach taken with B-Trees in the book:

*Introduction to Algorithms, 3rd Edition - Cormen, 2011*

The implementation stores all the b-tree nodes in a binary file, and constantly writes and reads from the said file in order to add new ones and to update the nodes information.

The file created is organized and treated like an array, the tree keeps track of the numbers of nodes it has and where each one is, so, when a new key/item is added a new position in the file/array is given/added.

To access directly to the node in the file fseek() is used to "jump" directly to the node location, witch gives the file "the array like properties".

This implementation lacks the delete method and other small but usefull methods, like get min/max item, etc... (in progress)

Methods Implementaded:

 - Crete B-Tree
 - Insertion
 - Search
 - Print tree
 
Insertion:

When an item is being inserted in a leaf that is curently full, before inserting, a check is made while trasversing down the tree to see of the is the previous nodes are also full and if yes, they get splitted, this garantees that when a full leaf is reached and a split is aplyied the parent node is able to recieve the key that will be promoted.

Search:

Starts in the root, compares its keys values with the one to be found, if not there, dives further in the tree following the right child.
The process e repeated until a leaf without containning the item is reached or the item is found in witch case it returns its position in the file.

Print:

A queue is created, it will hold all the tree nodes in a given order for later printing.

The tree is printed from the root to the leafs, to achieve this, the root is inserted in the queue, then her children and then the children´s children until all the nodes are covered. At the end of each "level" of the tree a marker is inserted in the queue to mark the end of that level

While the above process is happenning the items already in queue are being removed and its keys printed.

Note: I will also provide a queue implementation.
