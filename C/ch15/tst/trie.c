#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "trie.h"

// NOTE: Terminology:
//
//   A key is a C string, terminated with a 0-byte.
//   You should make no assumptions about the valid characters for the string
//   or should not try to determine that a key is a valid English word.
//
//   Example (valid) keys:
//
//       "a"
//       "Aa"
//       "te@st"
//       "Te st"
//       "te st"
//
//    (these are all distinct keys)

// The structure representing the trie
struct trie_data_t {
    trie_pos_t start;
};

// A structure representing a trie node
struct trie_node_t {
    char key;
    void *val;
    char *fullkey;
    trie_pos_t left;
    trie_pos_t right;
    trie_pos_t mid;
    trie_pos_t parent;
};

/* Helper function to recursively walk each element */
bool trie_walk_nodes(trie_t trie, trie_pos_t head, trie_walk_t walkfunc, void * priv) {
    if (head == NULL) { return true; }

    if (head->val != NULL) {    // we hit a full key!
        if (!walkfunc(trie, head, head->fullkey, priv)) { return false; }
    }

    if (!trie_walk_nodes(trie, head->mid, walkfunc, priv)) { return false; }
    if (!trie_walk_nodes(trie, head->left, walkfunc, priv)) { return false; }
    if (!trie_walk_nodes(trie, head->right, walkfunc, priv)) { return false; }

    return true;
}

/// Visit every key in the trie
///   Calls walkfunc for every key
///   - If walkfunc returns true, the tree walking continues;
///   - If walkfunc returns false, the tree walking stops immediately
///
/// priv is passed to the walking function; trie_walk does not use it.
///
/// Returns true if the walkfunc never returned false or if there were
/// no keys in the trie
///
/// Returns false if the walkfunc returned false.
///
bool trie_walk (trie_t trie, trie_walk_t walkfunc, void * priv) {
    if (trie->start == NULL) { return true; }
    return trie_walk_nodes(trie, trie->start, walkfunc, priv);
}

void trie_free_node(trie_pos_t node, trie_free_t freefunc) {
    if (node == NULL) { return; }

    trie_free_node(node->mid, freefunc);
    trie_free_node(node->left, freefunc);
    trie_free_node(node->right, freefunc);

    if ((node->val != NULL) && (freefunc != NULL)) {
        freefunc(node->val);
    }

    free(node->fullkey);
    node->val = NULL;
    node->mid = NULL;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    free(node);
    return;
}

/// Free trie
/// If freefunc is not NULL, calls freefunc for every void * value
/// associated with a key.
void trie_destroy (trie_t trie, trie_free_t freefunc) {
    trie_free_node(trie->start, freefunc);
    free(trie);
}

/// Get value associated with a key
/// NOTE: the pos was obtained by a call to trie_insert or trie_find
void * trie_get_value (const trie_t trie, trie_pos_t pos) {
    return pos->val;
}

/// Set value associated with a key
/// NOTE: the pos was obtained by a call to trie_insert or trie_find.
void trie_set_value (trie_t trie, trie_pos_t pos, void * value) {
    pos->val = value;
}

/* Does the node in question have any children? */
bool trie_check_children(trie_pos_t head) {
    return (head->left || head->right || head->mid ? true : false);
}

/// Create a new empty trie
trie_t trie_new() {
    trie_t new = (trie_t)malloc(sizeof(struct trie_data_t));
    if (new == NULL) { return TRIE_INVALID; }

    new->start = NULL;      // initialize to empty trie
    return new;
}

/* Helper functin to generate a new node instance */
trie_pos_t trie_new_node(const char src, void *newval) {
    trie_pos_t newbie = (trie_pos_t)malloc(sizeof(struct trie_node_t));
    if (newbie == NULL) { return NULL; }

    newbie->left = NULL;
    newbie->right = NULL;
    newbie->mid = NULL;
    newbie->parent = NULL;
    newbie->fullkey = NULL;

    newbie->key = src;
    newbie->val = newval;

    return newbie;
}

/* Recursive count of full strings (keys) in our trie. We check for the
   presence of a 'val', and if there increment */
unsigned int trie_size_nodes(trie_pos_t head) {
    unsigned int count = 0;

    if (head == NULL) { return count; }
    if (head->val != NULL) { ++count; }
    count += trie_size_nodes(head->mid);
    count += trie_size_nodes(head->right);
    count += trie_size_nodes(head->left);
    return count;
}

/// Return the number of keys in the trie
unsigned int trie_size (const trie_t trie) {
    if (trie->start == NULL) { return 0; }
    return trie_size_nodes(trie->start);
}

/* Helper function to recursively find a specific node */
trie_pos_t trie_find_node(trie_pos_t head, const char *src) {
    if (head == NULL) { return TRIE_INVALID_POS; }
    if ((*(src+1) == '\0') && (*src == head->key) && (head->val != NULL)) {
        return head; // we found it?!
    }

    if (*src < head->key) { return trie_find_node(head->left, src); }
    if (*src == head->key) { return trie_find_node(head->mid, src+1); }
    if (*src > head->key) { return trie_find_node(head->right, src); }
    return head;
}

/// Find a key in a trie
/// Returns the position or TRIE_INVALID_POS if the key could not be found.
trie_pos_t trie_find (const trie_t trie, const char * key) {
    if (trie->start == NULL) { return TRIE_INVALID_POS; }
    return trie_find_node(trie->start, key);
}

/* using ternary search tree (TST) after reading CH 15: Radix Search in Algorithms in C (Sedgewick) */
trie_pos_t trie_insert_node(trie_pos_t head, const char *src, const char *fullkey, void *theval, trie_pos_t *newpos) {
    if (*src == '\0') { return head; }

    if (fullkey == NULL) { return TRIE_INVALID_POS; }

    if (head == NULL) { head = trie_new_node(*src, NULL); }  // we know our node is blank, so insert!

    if (*src < head->key) {
        head->left = trie_insert_node(head->left, src, fullkey, theval, newpos);
        if (head->left != NULL) { head->left->parent = head; }
        return head;
    }

    if (*src == head->key) {
        head->mid = trie_insert_node(head->mid, src+1, fullkey, theval, newpos);
        if (head->mid != NULL) { head->mid->parent = head; }
    }

    if (*src > head->key) {
        head->right = trie_insert_node(head->right, src, fullkey, theval, newpos);
        if (head->right != NULL) { head->right->parent = head; }
        return head;
    }

    if ((*(src+1) == '\0') && (head->val == NULL)) {
        head->val = theval;
        head->fullkey = (char *)calloc(strlen(fullkey)+1, sizeof(char));
        if (head->fullkey == NULL) { return TRIE_INVALID_POS; }
        strcpy(head->fullkey, fullkey);

        if ((head->key == *src) && (newpos != NULL)) { (*newpos) = head; }
    }

    return head;
}

/* Helper function to find the spot we'd inserted a rotated node! */
/* When we're handling removes, we usually have to 'shake out the trie' to put the links
   back into their proper order. The beauty of using a TST is that we know anything
   to the left of a node is less than the root, anything to the right is greater than the root.
   Using those principles, we just need to find where the 're-sorted' node falls into place */
trie_pos_t trie_find_spot(trie_pos_t head, const char key) {
    trie_pos_t tester;
    if ((key == '\0') || (head == NULL)) { return head; }
    if (key < head->key) { tester->left = trie_find_spot(head->left, key); if (tester->left == NULL) { return head; } else { return tester->left; } }
    if (key > head->key) { tester->right = trie_find_spot(head->right, key); if (tester->right == NULL) { return head; } else {  return tester->right; } }

    return head;
}

/// Insert a key in the trie;
///
///  Returns true if a new key was inserted, in which case the data
///  value associated with the new key is set to newval;
///
///  Returns false if the key already existed, in which case
///  its data value IS NOT UPDATED.
///
///  In both cases, newpos is set to the position of the new (or existing)
///  key in the trie, *provided* newpos is not NULL.
///
///  Note:
///  A position only remains valid until the next change (insertion or removal)
///  in the trie.
///
bool trie_insert (trie_t trie, const char * str, void * newval,
      trie_pos_t * newpos) {
    bool ret = false;
    size_t count = 0, after = 0;

    trie_pos_t random = trie_find(trie, str);
    if (random != TRIE_INVALID_POS) {
        if (newpos != NULL) {
            newpos = &random;
            return false;
        }
    }

    count = trie_size(trie);
    trie->start = trie_insert_node(trie->start, str, str, newval, newpos);
    after = trie_size(trie);
    if ((count < after)) { ret = true; } else { ret = false; }

    return ret;
}

/* Very large, inefficient function to handle the actual key removal. Our data structure is amazingly fast
   for insertion and searching, but sucks for removal given the complexity of the links. The beauty of the TST is
   that we know anything to the left of a node is < node, anything to the right is > node, and everything in the
   middle is the rest of the key.

   With that logic, we basically try to 're-sort' the affected keys after removing one. There is a lot of nested
   logic here and with more time a refactor would be ideal.

   The general algorithm passes the current node (found) and its parent into this function. Based on the location
   of both found + parent, we have to deduce whether it's a simple update or a complex update. Handling the root
   node was difficult, but the lower-level changes are by far the most intricate. If you have something like

                                    t
                                    |
                                    e
                                    |
                                    s
                                  / | \
                                 d  t  t

    ...any you wnat to remove 'test', you have to handle who gets moved up and who gets re-sorted! In our
    implementation, d would be rotated into place of s, t would stay where it is and we'd remove 'st' as long as
    no substr existed (like tes). If that exists, all this is simplified and only the last 't' is removed under
    's' since the other keys are intact.

    It's overly complex, but works for most cases. Please see the README.txt for more information

*/
trie_pos_t trie_remove_key(trie_t thetrie, trie_pos_t parent, trie_pos_t found, const char *src, size_t count) {
    /* if we're on a branch that's less than parent, make the swap */
    if (parent->left == found) {
        if (found->right != NULL) {
            parent->left = found->right;
            found->right->parent = parent;
            if (found->left != NULL) { found->right->left = found->left; }
            found->left = NULL;
            found->right = NULL;
        } else if (found->left != NULL) {
            parent->left = found->left;
            found->left->parent = parent;
            found->left = NULL;
        } else { parent->left = NULL; }

        return found;
    }

    /* if we're on a branch that's greater than parent, make the swap */
    if (parent->right == found) {
        if (found->left != NULL) {
            parent->right = found->left;
            found->left->parent = parent;
            if (found->right != NULL) { found->left->right = found->right; }
            found->left = NULL;
            found->right = NULL;
        } else if (found->right != NULL) {
            parent->right = found->right;
            found->right->parent = parent;
            found->right = NULL;
        } else { parent->right = NULL; }

        return found;
    }

    // the root node, yikes! This one requires more logic, particularly with multiple links
    if (parent->parent == NULL) {
        if (parent->val != NULL) {
            parent->val = NULL;
            free(parent->fullkey);
            parent->fullkey = NULL;
            return TRIE_INVALID_POS;
        }

        if (parent->right != NULL) {
            if (parent->left != NULL) {
                parent->left->parent = trie_find_spot(parent->right, parent->left->key);
                if (parent->left->key < parent->left->parent->key) { parent->left->parent->left = parent->left; }
                else if (parent->left->key > parent->left->parent->key) { parent->left->parent->right = parent->left; }
            }
            thetrie->start = parent->right;
            parent->right->parent = NULL;   // new root
            parent->left = NULL;
            parent->right = NULL;
        }

        if (parent->left != NULL) {
            thetrie->start = parent->left;
            parent->left->parent = NULL;    // new root
            parent->left = NULL;
            parent->right = NULL;
        }

        return parent;
    }

    /* most complex case, both left and right have links. we have to be careful here to take into account
       the re-sorted order for nodes. */
    if (parent->right != NULL) {
        if (parent->left != NULL) {                     // move these to a function somehow
            if (parent->parent->left == parent) {       // update the left side of parent
                parent->right->parent = parent->parent;
                parent->parent->left = parent->right;
                parent->left->parent = trie_find_spot(parent->right, parent->left->key);
                if (parent->left->key < parent->left->parent->key) { parent->left->parent->left = parent->left; }
                else if (parent->left->key > parent->left->parent->key) { parent->left->parent->right = parent->left; }
                parent->left = NULL;
                parent->right = NULL;
            } else if (parent->parent->right == parent) {       // update the left side of parent
                parent->left->parent = parent->parent;
                parent->parent->right = parent->left;
                parent->right->parent = trie_find_spot(parent->left, parent->right->key);
                if (parent->right->key < parent->right->parent->key) { parent->right->parent->left = parent->right; }
                else if (parent->right->key > parent->right->parent->key) { parent->right->parent->right = parent->right; }
                parent->left = NULL;
                parent->right = NULL;
            } else {
                parent->left->parent = parent->parent;
                parent->parent->mid = parent->left;
                parent->right->parent = trie_find_spot(parent->left, parent->right->key);
                if (parent->right->key < parent->right->parent->key) { parent->right->parent->left = parent->right; }
                else if (parent->right->key > parent->right->parent->key) { parent->right->parent->right = parent->right; }
                parent->left = NULL;
                parent->right = NULL;
            }
        } else {
            if (count == 0) {   // end of string
                if (parent->val != NULL) {
                    parent->val = NULL;
                    free(parent->fullkey);
                    parent->fullkey = NULL;
                    if (parent->mid == NULL) {
                        if (parent->parent->mid == found) {
                            parent->parent->mid = parent->right;
                        } else {
                            parent->parent->right = parent->right;
                        }
                        parent->right->parent = parent->parent;
                        parent->right = NULL;
                        return parent;
                    } else { return TRIE_INVALID_POS; }
                }
            } else if (strlen(src) - count - 1 > 0) {      // found a str with similar parent
                if (parent->val == NULL) {
                    parent->parent->mid = parent->right;
                    parent->right->parent = parent->parent;
                }
            } else {
                if (parent->val == NULL) {
                    parent->right->parent = parent->parent;
                    if (parent->parent->left == parent) { parent->parent->left = parent->right; }
                    else if (parent->parent->right == parent) { parent->parent->right = parent->right; }
                }
            }
        }
        if (parent->val != NULL) { parent->mid = NULL; return found; }
        parent->right = NULL;
        return parent;
    }

    /* simplified case, only the left exists (because we would have caught the right side above) */
    if (parent->left != NULL) {
        if (count == 0) {   // end of string
            if (parent->val != NULL) {
                parent->val = NULL;
                free(parent->fullkey);
                parent->fullkey = NULL;
                if (parent->mid == NULL) {
                    if (parent->parent->mid == found) {
                        parent->parent->mid = parent->left;
                    } else {
                        parent->parent->left = parent->left;
                    }
                    parent->left->parent = parent->parent;
                    parent->left = NULL;
                    return parent;
                } else { return TRIE_INVALID_POS; }
            }
        } else if (strlen(src) - count - 1 > 0) {      // found a str with similar parent
            if (parent->val == NULL) {
                parent->parent->mid = parent->left;
                parent->left->parent = parent->parent;
            }
        } else {
            if (parent->val == NULL) {
                parent->left->parent = parent->parent;
                if (parent->parent->left == parent) { parent->parent->left = parent->left; }
                else if (parent->parent->right == parent) { parent->parent->right = parent->left; }
            }
        }
        if (parent->val != NULL) { parent->mid = NULL; return found; }
        parent->left = NULL;
        return parent;
    }

    /* if only the middle exists and the parent points to current node, then we're inside a substr */
    if (parent->mid == found) {
        if (parent->val == NULL) {
            found = trie_remove_key(thetrie, parent->parent, parent, src, ++count);
            return found;
        } else {
            parent->mid = NULL;
            return found;
        }
    }

    /* otherwise, we're actually a full string without any depedencies */
    if (parent->mid != NULL) {
        if (parent->val == NULL) {
            found = trie_remove_key(thetrie, parent->parent, parent, src, ++count);
            return found;
        } else {
            parent->val = NULL;
            free(parent->fullkey);
            parent->fullkey = NULL;
        }
    }

    return TRIE_INVALID_POS;
}

/// Remove a key from a trie
/// Returns false if the key could not be found
///
/// Returns true if the key was removed, and sets *data to the data value
/// associated with the key (so it can be properly disposed of by the user,
/// if needed).
bool trie_remove (trie_t trie, const char * key, void ** data) {
    if (trie->start == NULL) { return false; }
    if ((strcmp(key, "") == 0) || (key == NULL)) { return false; }

    trie_pos_t found = trie_find(trie, key);
    if (found == NULL) { return false; }    // key not found

    if (found->val != NULL) {
        if (data != NULL) { (*data) = found->val; }
    } else { return false; }    // found a substr of the key, not the key itself

    if (!trie_check_children(found)) {
        found = trie_remove_key(trie, found->parent, found, key, 1);    // bubble up
    } else {
        found = trie_remove_key(trie, found, found, key, 0);
    }

    if (found != TRIE_INVALID_POS) {
        if (trie->start == found) { trie->start = NULL; }
        trie_free_node(found, NULL);
    } else { free(found); }

    return true;
}
