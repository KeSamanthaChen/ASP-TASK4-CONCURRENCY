#include <stddef.h>
#include <iostream>

#define PAD 64

// using mutex
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
	char padding[PAD]; //what's this?
	struct Node_HM_t* m_next; //pointer to next node in the bucket
} Node_HM;


// defining a bucket in the hashmap
// what is this for, the header?
typedef struct List_t {
	Node_HM* sentinel; //list of nodes in a bucket
} List;


// defining the hashmap
typedef struct hm_t {
    List** buckets; //list of buckets in the hashmap
    long n_buckets;
} HM;


// allocate a hashmap with given number of buckets
// https://stackoverflow.com/questions/39397322/declaring-a-2d-array-using-double-pointer
HM* alloc_hashmap(size_t n_buckets) {
    HM new_hm;
    new_hm.n_buckets = n_buckets;
    new_hm.buckets = new List*[n_buckets]; // need delete
    for (int i=0; i<n_buckets; i++) {
        // do some thing
        new_hm.buckets[i]->sentinel = new Node_HM_t;
        new_hm.buckets[i]->sentinel->m_val = i; // kind of like the key
    }
    return &new_hm;
}

//free a hashamp
void free_hashmap(HM* hm) {
    for (int i=0; i<hm->n_buckets; i++) {
        // delete sentinel is different from delete hm->buckets[i]
        Node_HM* current_node = hm->buckets[i]->sentinel;
        Node_HM* pointer;
        while(current_node != NULL) {
            // when insert value, this will be using "new", so need to delete there
            pointer = current_node;
            current_node = current_node->m_next;
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
    //if the value already here? duplicated allow
    Node_HM* current_node = hm->buckets[key_value]->sentinel;
    while (current_node->m_next!=NULL) {
        current_node = current_node->m_next;
    }
    //end of list
    Node_HM* new_node = new Node_HM;
    new_node->m_next = NULL;
    new_node->m_val = val;
    current_node->m_next = new_node;
    return 0;
}

//remove val from the hm, if it exist and return 0 if successful
//return 1 if item is not found
int remove_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* current_node = hm->buckets[key_value]->sentinel->m_next;
    Node_HM* previous_node = hm->buckets[key_value]->sentinel;
    while (current_node!=NULL) {
        if (current_node->m_val == val) {
            // delete and reconnect
            previous_node->m_next = current_node->m_next;
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
    Node_HM* current_node = hm->buckets[key_value]->sentinel->m_next;
    while (current_node!=NULL) {
        if (current_node->m_val == val)
            return 0;
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
        Node_HM* current_node = hm->buckets[i]->sentinel->m_next;
        while(current_node!=NULL) {
            std::cout << " - " << current_node->m_val << std::flush;
        }
        std::cout << std::endl;
    }
}