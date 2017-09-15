#pragma once

#include <stdbool.h>

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
struct trie_data_t;
typedef struct trie_data_t * trie_t;

#define TRIE_INVALID ((trie_t) 0)

// A structure representing a trie node
struct trie_node_t;
typedef struct trie_node_t * trie_pos_t;

#define TRIE_INVALID_POS ((trie_pos_t) 0)

/// Function which gets called when a trie node is deleted as part of
/// the trie_destroy function.
typedef void (*trie_free_t) (void * data);

/// Function which gets called when traversing the trie
/// The function should return true
/// priv (the priv argument to trie_walk) is passed to trie_walk_t
typedef bool (*trie_walk_t) (trie_t trie,
       trie_pos_t pos, const char * key, void * priv);

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
bool trie_walk (trie_t trie, trie_walk_t walkfunc, void * priv);

/// Free trie
/// If freefunc is not NULL, calls freefunc for every void * value
/// associated with a key.
void trie_destroy (trie_t trie, trie_free_t freefunc);

/// Get value associated with a key
/// NOTE: the pos was obtained by a call to trie_insert or trie_find
void * trie_get_value (const trie_t trie, trie_pos_t pos);

/// Set value associated with a key
/// NOTE: the pos was obtained by a call to trie_insert or trie_find.
void trie_set_value (trie_t trie, trie_pos_t pos, void * value);

/// Create a new empty trie
trie_t trie_new ();

/// Return the number of keys in the trie
unsigned int trie_size (const trie_t trie);

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
      trie_pos_t * newpos);


/// Find a key in a trie
/// Returns the position or TRIE_INVALID_POS if the key could not be found.
trie_pos_t trie_find (const trie_t trie, const char * key);

/// Remove a key from a trie
/// Returns false if the key could not be found
///
/// Returns true if the key was removed, and sets *data to the data value
/// associated with the key (so it can be properly disposed of by the user,
/// if needed).
bool trie_remove (trie_t trie, const char * key, void ** data);

