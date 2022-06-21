#include <stddef.h>
#include <iostream>
#include <atomic>
#include <vector>
#include "chashmap.h"

#define PAD 64

// class Node<KeyType> {
//     KeyType key;
//     Node *next;
//     Node (KeyType key) {
//         this.key = key;
//     }
// }

//define a node in the hashmap
typedef struct Node_HM_t {
	long m_val; //value of the node
	char padding[PAD];
	std::atomic<Node_HM_t*> m_next; //pointer to next node in the bucket //may be atonic here
} Node_HM;

// class List<KeyType> {
//     Node<KeyType> *head;
//     Node<KeyType> *tail;
//     List() {
//         head = new Node<KeyType> ();
//         tail = new Node<KeyType> ();
//         head.next = tail;
//     }
// }

typedef struct List_t {
	std::atomic<Node_HM_t*> sentinel; //head
    std::atomic<Node_HM_t*> tail; // tail
} List;

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
    //init list
    for (int i=0; i<n_buckets; i++) {
        new_hm->buckets[i] = new List[1];
        new_hm->buckets[i][0].sentinel = new Node_HM;
        new_hm->buckets[i][0].tail = new Node_HM;
        new_hm->buckets[i][0].sentinel.load(std::memory_order_relaxed)->m_next = new_hm->buckets[i][0].tail.load(std::memory_order_relaxed);
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
    delete hm; //am I forget that...
}


// is_marked_reference
// get_marked_reference
// get_unmarked_reference
// https://github.com/sysprog21/concurrent-ll/blob/master/src/lockfree/list.c
inline bool is_marked_reference(void *i)
{
    return (bool) ((uintptr_t) i & 0x1L);
}

inline Node_HM *get_unmarked_reference(void *w)
{
    return (Node_HM *) ((uintptr_t) w & ~0x1L);
}

inline Node_HM *get_marked_reference(void *w)
{
    return (Node_HM *) ((uintptr_t) w | 0x1L);
}


// private Node *List::search (KeyType search_key, Node **left_node) {
//     Node *left_node_next, *right_node;
// search_again:
//     do {
//         Node *t = head;
//         Node *t_next = head.next;
//     /* 1: Find left_node and right_node */
//         do {
//             if (!is_marked_reference(t_next)) {
//                 (*left_node) = t;
//                 left_node_next = t_next;
//             }
//             t = get_unmarked_reference(t_next);
//             if (t == tail) break;
//             t_next = t.next;
//         } while (is_marked_reference(t_next) || (t.key<search_key)); /*B1*/
//         right_node = t;
//         /* 2: Check nodes are adjacent */
//         if (left_node_next == right_node)
//             if ((right_node != tail) && is_marked_reference(right_node.next))
//                 goto search_again; /*G1*/
//             else
//                 return right_node; /*R1*/
//         /* 3: Remove one or more marked nodes */
//         if (CAS (&(left_node.next), left_node_next, right_node)) /*C1*/
//             if ((right_node != tail) && is_marked_reference(right_node.next))
//                 goto search_again; /*G2*/
//             else
//                 return right_node; /*R2*/
//     } while (true); /*B2*/
// }

Node_HM* search_item(HM* hm, long val, Node_HM** left_node) {
    long key_value = val%hm->n_buckets;
    Node_HM* left_node_next;
    Node_HM* right_node;
    std::atomic<Node_HM*> atomic_left_node;
    while (1) {
        Node_HM *t = hm->buckets[key_value][0].sentinel;
        Node_HM *t_next = hm->buckets[key_value][0].sentinel.load()->m_next;
        /* 1: Find left_node and right_node */
        while (is_marked_reference(t_next) || (t->m_val < val)) {
            if (!is_marked_reference(t_next)) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_reference(t_next);
            if (t == hm->buckets[key_value][0].tail) break;
            t_next = t->m_next;
        } /*B1*/
        right_node = t;

        /* 2: Check nodes are adjacent */
        if (left_node_next == right_node)
            if ((right_node != hm->buckets[key_value][0].tail) && is_marked_reference(right_node->m_next))
                continue; /*G1*/ // go to the first while(1)
            else
                return right_node; /*R1*/

        /* 3: Remove one or more marked nodes */
        atomic_left_node = (*left_node);
        if (std::atomic_compare_exchange_weak_explicit(&(atomic_left_node.load()->m_next), &left_node_next, right_node, std::memory_order_release, std::memory_order_relaxed)) /*C1*/
            if ((right_node != hm->buckets[key_value][0].tail) && is_marked_reference(right_node->m_next))
                continue; /*G2*/
            else
                return right_node; /*R2*/
    }
}


// public boolean List::insert (KeyType key) {
//     Node *new_node = new Node(key);
//     Node *right_node, *left_node;
//     do {
//         right_node = search (key, &left_node);
//         if ((right_node != tail) && (right_node.key == key)) /*T1*/
//             return false;
//         new_node.next = right_node;
//         if (CAS (&(left_node.next), right_node, new_node)) /*C2*/
//             return true;
//     } while (true); /*B3*/
// }

//insert val into the hm and return 0 if successful
//return 1 otherwise, e.g., could not allocate memory
int insert_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* new_node = new Node_HM;
    new_node->m_val = val;
    Node_HM* right_node;
    Node_HM* left_node;
    std::atomic<Node_HM*> atomic_left_node;

    while(1) {
        // right_node = search_item(hm, val, &left_node);
        right_node = search_item(hm, val, &left_node);
        if ((right_node != hm->buckets[key_value][0].tail) && (right_node->m_val == val)) //for duplication, the val already exists, no insertion performs
            return 1; // if right_node has the same value with new val, and right node is not the end of list
        new_node->m_next = right_node;
        atomic_left_node = left_node;
        if (std::atomic_compare_exchange_weak_explicit(&(atomic_left_node.load()->m_next), &right_node, new_node, std::memory_order_release, std::memory_order_relaxed)) /*C2*/
            // if the left_node.next == right_node, then the nodes are adjacent, could link the new node between them
            return 0;
    }/*B3*/
}




// public boolean List::delete (KeyType search_key) {
//     Node *right_node, *right_node_next, *left_node;
//     do {
//         right_node = search (search_key, &left_node);
//         if ((right_node == tail) || (right_node.key != search_key)) /*T1*/
//             return false;
//         right_node_next = right_node.next;
//         if (!is_marked_reference(right_node_next))
//             if (CAS (&(right_node.next), /*C3*/
//                 right_node_next, get_marked_reference (right_node_next)))
//                 break;
//     } while (true); /*B4*/
//     if (!CAS (&(left_node.next), right_node, right_node_next)) /*C4*/
//         right_node = search (right_node.key, &left_node);
//     return true;
// }

int remove_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    std::atomic<Node_HM*> right_node;
    Node_HM* right_node_next;
    Node_HM* left_node;
    std::atomic<Node_HM*> atomic_left_node;

    while (1) {
        right_node = search_item(hm, val, &left_node);
        if ((right_node == hm->buckets[key_value][0].tail) || (right_node.load(std::memory_order_relaxed)->m_val != val)) /*T1*/
            return 1;
        right_node_next = right_node.load(std::memory_order_relaxed)->m_next;
        if (!is_marked_reference(right_node_next))
            if (std::atomic_compare_exchange_weak_explicit(&(right_node.load()->m_next), /*C3*/
                &right_node_next, get_marked_reference(right_node_next), std::memory_order_release, std::memory_order_relaxed))
                break;
    }
    Node_HM* normal_right_node = right_node; // slightly change here..
    atomic_left_node = left_node;
    if (!std::atomic_compare_exchange_weak_explicit(&(atomic_left_node.load()->m_next), &normal_right_node, right_node_next, std::memory_order_release, std::memory_order_relaxed)) /*C4*/
        // what is this for...
        right_node = search_item(hm, right_node.load()->m_val, &left_node);
    return 0;
}

// public boolean List::find (KeyType search_key) {
//     Node *right_node, *left_node;
//     right_node = search (search_key, &left_node);
//     if ((right_node == tail) ||
//         (right_node.key != search_key))
//         return false;
//     else
//         return true;
// }

//check if val exists in hm, return 0 if found, return 1 otherwise
int lookup_item(HM* hm, long val) {
    long key_value = val%hm->n_buckets;
    Node_HM* right_node;
    Node_HM* left_node;
    right_node = search_item(hm, val, &left_node);
    if ((right_node == hm->buckets[key_value][0].tail) || (right_node->m_val != val))
        return 1; // if right_node's val != val and right_node is the tail, then not found...
    else
        return 0;
}

void print_hashmap(HM* hm) {
    for(int i=0;i<hm->n_buckets;i++) {
        std::cout << "Bucket " << i << std::flush;
        Node_HM* current_node = hm->buckets[i][0].sentinel.load();
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
//     insert_item(new_hm, 4);
//     remove_item(new_hm, 4);
//     std::cout<< lookup_item(new_hm, 4) << " Hello World" << std::endl;
//     print_hashmap(new_hm);

//     return 0;
// }

