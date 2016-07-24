//
// File:        pf_hashtable.h
// Description: PF_HashTable class interface
// Author:      Ding JinXiang
//
#ifndef PF_HASHTABLE_H
#define PF_HASHTABLE_H

#include "pf_internal.h"

//
// HashEntry - Hash table bucket entries
//
struct PF_HashEntry {
    PF_HashEntry *next;   // next hash table element or NULL
    PF_HashEntry *prev;   // prev hash table element or NULL
    int          fd;      // file descriptor
    PageNum      pageNum; // page number
    int          slot;    // slot of this page in the buffer
};

//
// PF_HashTable - allow search, insertion, and deletion of hash table entries
//
class PF_HashTable {
public:
    // Constructor
    PF_HashTable (int numBuckets);

    // Destructor
    ~PF_HashTable();

    // Set slot to the hash table entry for fd and pageNum
    RC  Find     (int fd, PageNum pageNum, int &slot);

    // Insert a hash table entry
    RC  Insert   (int fd, PageNum pageNum, int slot);

    // Delete a hash table entry
    RC  Delete   (int fd, PageNum pageNum);
private:
    // Hash function
    int Hash     (int fd, PageNum pageNum) const {
        return ((fd + pageNum) % numBuckets);
    }

    // Number of hash table buckets
    int numBuckets;

    // Hash table
    PF_HashEntry **hashTable;
};
#endif
