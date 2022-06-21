#include <stddef.h>
#include <iostream>
#include <atomic>
#include <vector>
#include "chashmap.h"

#define PAD 64

/*
Hashmap: list of buckets
bucket1 -> sentinel -> node1 -> node2 -> NULL
bucket2 -> sentinel -> node3 -> NULL
...
bucketN -> sentinel -> NULL
*/

//define a node in the hashmap
typedef struct Node_HM_t {
	long m_val; //value of the node
	char padding[PAD];
	std::atomic<Node_HM_t*> m_next; //pointer to next node in the bucket
} Node_HM;


// defining a bucket in the hashmap
// what is this for, the header?
typedef struct List_t {
	Node_HM* sentinel; //list of nodes in a bucket
    std::atomic<Node_HM*> tail;
} List;


// defining the hashmap
typedef struct hm_t {
    List** buckets; //list of buckets in the hashmap
    long n_buckets;
} HM;


// allocate a hashmap with given number of buckets
// https://stackoverflow.com/questions/936687/how-do-i-declare-a-2d-array-in-c-using-new
HM* alloc_hashmap(size_t n_buckets) {
    HM *new_hm = new HM;
    new_hm->n_buckets = n_buckets;
    new_hm->buckets = new List*[n_buckets]; // need delete
    for (int i=0; i<n_buckets; i++) {
        new_hm->buckets[i] = new List[1];
        new_hm->buckets[i][0].sentinel = new Node_HM;
        new_hm->buckets[i][0].sentinel->m_val = i; //// kind of like the key
        new_hm->buckets[i][0].tail = new_hm->buckets[i][0].sentinel;
    }
    return new_hm;
}


//free a hashamp
void free_hashmap(HM* hm) {
    for (int i=0; i<hm->n_buckets; i++) {
        // delete sentinel is different from delete hm->buckets[i]
        Node_HM* current_node = hm->buckets[i][0].sentinel;
        Node_HM* pointer;
        while(current_node != NULL) {
            // when insert value, this will be using "new", so need to delete there
            pointer = current_node;
            current_node = current_node->m_next;
            // how about m_next and padding?
            delete pointer;
        }
        delete hm->buckets[i];
    }
    delete hm->buckets;
}


//insert val into the hm and return 0 if successful
//return 1 otherwise, e.g., could not allocate memory
int insert_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* new_node = new Node_HM;
    new_node->m_next = NULL;
    new_node->m_val = val;
    Node_HM* nNULL = NULL;
    Node_HM* tail = NULL;
    Node_HM* tail_next = NULL;

    while (1) {
        tail = hm->buckets[key_value][0].tail.load(std::memory_order_relaxed);
        tail_next = hm->buckets[key_value][0].tail.load(std::memory_order_relaxed)->m_next;
        if (hm->buckets[key_value][0].tail.load(std::memory_order_relaxed)->m_next.compare_exchange_weak(nNULL, new_node)) {
            hm->buckets[key_value][0].tail.compare_exchange_weak(tail, new_node);
            return 0;
        }
    }
}


//remove val from the hm, if it exist and return 0 if successful
//return 1 if item is not found
int remove_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* current_node = hm->buckets[key_value][0].sentinel->m_next;
    Node_HM* previous_node = hm->buckets[key_value][0].sentinel;

    while (current_node!=NULL) {
        if (current_node->m_val == val) {
            // delete and reconnect
            previous_node->m_next = current_node->m_next.load();
            delete current_node;
            return 0;
        }
        // looping update
        previous_node = current_node;
        current_node = current_node->m_next;
    }
    return 1;
}


//check if val exists in hm, return 0 if found, return 1 otherwise
int lookup_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* current_node = hm->buckets[key_value][0].sentinel->m_next;
    while (current_node!=NULL) {
        if (current_node->m_val == val) {
            return 0;
        } 
        current_node = current_node->m_next;
    }
    return 1;
}


//print all elements in the hashmap as follows:
//Bucket 1 - val1 - val2 - val3 ...
//Bucket 2 - val4 - val5 - val6 ...
//Bucket N -  ...
void print_hashmap(HM* hm) {
    for(int i=0;i<hm->n_buckets;i++) {
        std::cout << "Bucket " << i << std::flush;
        Node_HM* current_node = hm->buckets[i][0].sentinel->m_next;
        while(current_node!=NULL) {
            std::cout << " - " << current_node->m_val << std::flush;
            current_node = current_node->m_next;
        }
        std::cout << std::endl;
    }
}

// int main()
// {
//     HM *new_hm = alloc_hashmap(3);
//     insert_item(new_hm, 5);
//     insert_item(new_hm, 5);
//     std::cout<< lookup_item(new_hm, 5) << " Hello World" << std::endl;
//     print_hashmap(new_hm);

//     return 0;
// }
