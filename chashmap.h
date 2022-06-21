// Reference: https://stackoverflow.com/questions/2744181/how-to-call-c-function-from-c
#include <stddef.h> // for NULL
#include <stdbool.h>
#define PAD 64

#ifdef __cplusplus
#include <atomic>
using namespace std;
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef struct Node_HM_t Node_HM;
typedef struct List_t List;
typedef struct hm_t HM;

EXTERNC HM* alloc_hashmap(size_t n_buckets);
EXTERNC void free_hashmap(HM* hm);
EXTERNC int insert_item(HM* hm, long val);
EXTERNC int remove_item(HM* hm, long val);
EXTERNC int lookup_item(HM* hm, long val);
EXTERNC void print_hashmap(HM* hm);
EXTERNC Node_HM* search_item(HM* hm, long val, atomic<Node_HM*>* left_node);

EXTERNC bool is_marked_reference(void *i);
EXTERNC Node_HM *get_unmarked_reference(void *w);
EXTERNC Node_HM *get_marked_reference(void *w);

#undef EXTERNC

// /*
// Hashmap: list of buckets
// bucket1 -> sentinel -> node1 -> node2 -> NULL
// bucket2 -> sentinel -> node3 -> NULL
// ...
// bucketN -> sentinel -> NULL
// */

// //define a node in the hashmap
// /*typedef struct Node_HM_t
// {
// 	long m_val; //value of the node
// 	char padding[PAD];
// 	struct Node_HM_t* m_next; //pointer to next node in the bucket
// } Node_HM;*/
// typedef struct Node_HM_t Node_HM;

// /*defining a bucket in the hashmap
// typedef struct List_t
// {
// 	Node_HM* sentinel; //list of nodes in a bucket
// } List;
// */
// typedef struct List_t List;

// /*defining the hashmap
// typedef struct hm_t
// {
//         List** buckets; //list of buckets in the hashmap
// } HM;
// */
// typedef struct hm_t HM;

// //allocate a hashmap with given number of buckets
// HM* alloc_hashmap(size_t n_buckets);

// //free a hashamp
// void free_hashmap(HM* hm);

// //insert val into the hm and return 0 if successful
// //return 1 otherwise, e.g., could not allocate memory
// int insert_item(HM* hm, long val);

// //remove val from the hm, if it exist and return 0 if successful
// //return 1 if item is not found
// int remove_item(HM* hm, long val);

// //check if val exists in hm, return 0 if found, return 1 otherwise
// int lookup_item(HM* hm, long val);

// //print all elements in the hashmap as follows:
// //Bucket 1 - val1 - val2 - val3 ...
// //Bucket 2 - val4 - val5 - val6 ...
// //Bucket N -  ...
// void print_hashmap(HM* hm);
