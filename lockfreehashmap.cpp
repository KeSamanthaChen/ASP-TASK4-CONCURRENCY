#include <stddef.h>
#include <iostream>
#include <atomic>
#include <vector>
#include "chashmap.h"

#define PAD 64

class List<KeyType> {
    Node<KeyType> *head;
    Node<KeyType> *tail;
    List() {
        head = new Node<KeyType> ();
        tail = new Node<KeyType> ();
        head.next = tail;
    }
}

class Node<KeyType> {
    KeyType key;
    Node *next;
    Node (KeyType key) {
        this.key = key;
    }
}

public boolean List::insert (KeyType key) {
    Node *new_node = new Node(key);
    Node *right_node, *left_node;
    do {
        right_node = search (key, &left_node);
        if ((right_node != tail) && (right_node.key == key)) /*T1*/
            return false;
        new_node.next = right_node;
        if (CAS (&(left_node.next), right_node, new_node)) /*C2*/
            return true;
    } while (true); /*B3*/
}


public boolean List::delete (KeyType search_key) {
    Node *right_node, *right_node_next, *left_node;
    do {
        right_node = search (search_key, &left_node);
        if ((right_node == tail) || (right_node.key != search_key)) /*T1*/
            return false;
        right_node_next = right_node.next;
        if (!is_marked_reference(right_node_next))
            if (CAS (&(right_node.next), /*C3*/
                right_node_next, get_marked_reference (right_node_next)))
            break;
    } while (true); /*B4*/
    if (!CAS (&(left_node.next), right_node, right_node_next)) /*C4*/
        right_node = search (right_node.key, &left_node);
    return true;
}

public boolean List::find (KeyType search_key) {
    Node *right_node, *left_node;
    right_node = search (search_key, &left_node);
    if ((right_node == tail) ||
        (right_node.key != search_key))
        return false;
    else
        return true;
}

private Node *List::search (KeyType search_key, Node **left_node) {
    Node *left_node_next, *right_node;
search_again:
    do {
        Node *t = head;
        Node *t_next = head.next;
    /* 1: Find left_node and right_node */
        do {
            if (!is_marked_reference(t_next)) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_reference(t_next);
            if (t == tail) break;
            t_next = t.next;
        } while (is_marked_reference(t_next) || (t.key<search_key)); /*B1*/
        right_node = t;
        /* 2: Check nodes are adjacent */
        if (left_node_next == right_node)
            if ((right_node != tail) && is_marked_reference(right_node.next))
                goto search_again; /*G1*/
            else
                return right_node; /*R1*/
        /* 3: Remove one or more marked nodes */
        if (CAS (&(left_node.next), left_node_next, right_node)) /*C1*/
            if ((right_node != tail) && is_marked_reference(right_node.next))
                goto search_again; /*G2*/
            else
                return right_node; /*R2*/
    } while (true); /*B2*/
}