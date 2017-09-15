#include "trie.h"

#include <CUnit/Basic.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define CONCUR 6
#define MAX_STRING 250

static unsigned int countfunc_value = 0;

static void countfunc_free (void * data)
{
   ++countfunc_value;
}

static bool sebtest(trie_t t, trie_pos_t pos, const char * key,
      void * priv)
{
//    printf("key: %c, val: %ld, full: %s, node: %p, left: %p, mid: %p, right: %p, parent: %p\n", pos->key, (long)pos->val, pos->fullkey, (void *)pos, (void *)pos->left, (void *)pos->mid, (void *)pos->right, (void *)pos->parent);
   return true;
}

static void test_create_destroy()
{
   trie_t tries[CONCUR];
   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      tries[loop] = trie_new();
      CU_ASSERT_PTR_NOT_NULL(tries[loop]);
   }

   countfunc_value = 0;

   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      // Test both with freefunc and without
      trie_destroy (tries[loop], (loop & 0x1 ? countfunc_free : NULL));
   }

   CU_ASSERT_EQUAL(countfunc_value, 0);

}

static void test_insert()
{
   trie_t tries[CONCUR];
   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      tries[loop] = trie_new();
      CU_ASSERT_PTR_NOT_NULL(tries[loop]);
   }

   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      CU_ASSERT_TRUE(trie_insert(tries[loop], "test", (void*) 2, NULL));
      // 2nd insert should return false
      CU_ASSERT_FALSE(trie_insert(tries[loop], "test", (void*) 2, NULL));

      CU_ASSERT_TRUE(trie_insert(tries[loop], "another", (void*) 3, NULL));
      CU_ASSERT_TRUE(trie_insert(tries[loop], "test2", (void*) 4, NULL));
   }

   // Count should be 3
   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      CU_ASSERT_EQUAL(trie_size(tries[loop]), 3);
   }

   for (unsigned int loop=0; loop<CONCUR; ++loop)
   {
      // Test both with freefunc and without
      trie_destroy (tries[loop], NULL);
   }
}

static void test_size()
{

}

static uintptr_t hash_string (const char * s)
{
   if (!s)
      return 0;

   uintptr_t v = 0;
   while (*s)
   {
      v += *s++;
   }
   return v;
}

static void generate_random_string (char * buf, unsigned int maxlen)
{
   unsigned int s = (rand() % (maxlen-1))+1;
   for (unsigned int i=0; i<s; ++i)
   {
      buf[i]='a'+(rand() % 26);
   }
   buf[s]=0;
}

static void test_insert_random ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL(t);

   char buf[MAX_STRING+1];
   unsigned int inserted =0;
   for (unsigned int i=0; i<1000; ++i)
   {
      generate_random_string(buf, sizeof(buf));
      bool b = trie_insert (t, buf, (void*) hash_string(buf), NULL);
      if (b)
         ++inserted;
   }

   unsigned int count = trie_size(t);
   CU_ASSERT_EQUAL(count, inserted);

   // Check destroy is working
   countfunc_value = 0;
   trie_destroy(t, countfunc_free);
   CU_ASSERT_EQUAL(countfunc_value, count);
}


static void test_set_get ()
{
   trie_t t = trie_new ();
   CU_ASSERT_PTR_NOT_NULL_FATAL(t);

   // Insert single element
   trie_pos_t pos;
   CU_ASSERT_FATAL(trie_insert (t, "a", (void*) 0x1234, &pos));
   CU_ASSERT_PTR_NOT_NULL(pos);

   CU_ASSERT_EQUAL((void*) 0x1234, trie_get_value(t, pos));

   trie_set_value(t, pos, (void*) 0x1235);
   CU_ASSERT_EQUAL((void*) 0x1235, trie_get_value(t, pos));

   trie_destroy(t, NULL);
}

static void test_find ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL_FATAL(t);

   const char * strings[] = {"aaa", "aab", "aac", "aacd" };
   const unsigned int count =sizeof(strings)/sizeof(strings[0]);

   for (unsigned int i=0; i<count; ++i)
   {
      const char * buf = strings[i];
      trie_pos_t pos;
      bool b = trie_insert (t, buf, (void*) hash_string(buf), &pos);
      CU_ASSERT_TRUE(b);
      CU_ASSERT_EQUAL(hash_string(buf), trie_get_value(t, pos));
   }

   CU_ASSERT_PTR_NULL(trie_find(t, "aaaaaaaaaaaaaa"));

   // Find all of them
   for (unsigned int i=0; i<count; ++i)
   {
      trie_pos_t pos = trie_find (t, strings[i]);
      CU_ASSERT_PTR_NOT_NULL(pos);
      if (pos)
      {
         CU_ASSERT_EQUAL(hash_string(strings[i]), trie_get_value(t,pos));
      }
   }

   unsigned int inserted = trie_size(t);
   CU_ASSERT_EQUAL(count, inserted);

   // Check destroy is working
   countfunc_value = 0;
   trie_destroy(t, countfunc_free);
   CU_ASSERT_EQUAL(countfunc_value, count);
}


static bool test_walk_walker1 (trie_t t, trie_pos_t pos, const char * key,
      void * priv)
{
   uintptr_t * data = (uintptr_t *) priv;

   uintptr_t val = hash_string(key);
   *data += val;
   CU_ASSERT_EQUAL(trie_get_value(t, pos), val);
   return true;
}

static bool test_walk_walker2 (trie_t t, trie_pos_t pos, const char * key,
      void * priv)
{
   uintptr_t * count = (uintptr_t*) priv;
   CU_ASSERT_NOT_EQUAL(*count, 0);
   if (*count)
      --(*count);

   return *count;
}

static bool test_walk_walker3 (trie_t t, trie_pos_t pos, const char * key,
      void * priv)
{
   CU_ASSERT_EQUAL(priv, (void*) 0x1234);
   return true;
}

static void test_walk ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL(t);

   uintptr_t countdown = 0;

   // Test walker for empty tree; Should return true
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker2, &countdown));

   const char * test_strings[] = {"aaa", "aab", "aac", "aacd",
      "test", "aaaa"};
   const unsigned int string_count =
      sizeof(test_strings)/sizeof(test_strings[0]);

   // calculate hash
   uintptr_t expected = 0;
   for (unsigned int i=0; i<string_count; ++i)
   {
      const char * buf = test_strings[i];
      trie_pos_t pos;
      bool b = trie_insert (t, buf, (void*) hash_string(buf), &pos);
      expected += hash_string(buf);
      CU_ASSERT_TRUE(b);
      CU_ASSERT_EQUAL(hash_string(buf), trie_get_value(t, pos));
   }

   unsigned int inserted = trie_size(t);
   CU_ASSERT_EQUAL(string_count, inserted);

   // Test if priv value is properly passed
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker3, (void*) 0x1234));

   uintptr_t hash = 0;
   // Walk full, calculating hash
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker1, &hash));
   CU_ASSERT_EQUAL(hash, expected);

   // Test partial walk
   countdown = 3;
   CU_ASSERT_FALSE(trie_walk(t, test_walk_walker2, &countdown));
   CU_ASSERT_EQUAL(countdown, 0);

   // Check destroy is working
   countfunc_value = 0;
   trie_destroy(t, countfunc_free);
   CU_ASSERT_EQUAL(countfunc_value, string_count);
}

static void test_remove_fixed ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL(t);

   uintptr_t countdown = 0;

   // Test walker for empty tree; Should return true
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker2, &countdown));

   const char * test_strings[] = {"aaa", "aab", "aac", "aacd",
      "test", "aaaa"};
   unsigned int string_count =
      sizeof(test_strings)/sizeof(test_strings[0]);

   for (unsigned int i=0; i<string_count; ++i)
   {
      CU_ASSERT_TRUE(trie_insert(t, test_strings[i],
               (void*) hash_string(test_strings[i]), NULL));
   }

   // Sanity check
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Make sure we can't remove nodes without value even if the subkey is
   // present in the tree.
   CU_ASSERT_FALSE(trie_remove(t, "aa", NULL));
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Remove a descendant and check value
   void * data = 0;
   CU_ASSERT_TRUE(trie_remove(t, "aaaa", &data));
   --string_count;
   CU_ASSERT_EQUAL(data, (void*) hash_string("aaaa"));

   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Remove missing one
   CU_ASSERT_FALSE(trie_remove(t, "aaaa", NULL));

   // Remove remaining
   CU_ASSERT_TRUE(trie_remove(t, "test", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "aacd", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "aaa", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "aac", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "aab", NULL));

   CU_ASSERT_EQUAL(trie_size(t), 0);

   // Check destroy is working
   trie_destroy(t, NULL);
}

static void test_remove_sebtest ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL(t);

   uintptr_t countdown = 0;

   // Test walker for empty tree; Should return true
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker2, &countdown));

   const char * test_strings[] = {"ttt", "ttm", "ttd", "ttdb",
      "asdf", "tttt"};
   unsigned int string_count =
      sizeof(test_strings)/sizeof(test_strings[0]);

   for (unsigned int i=0; i<string_count; ++i)
   {
      CU_ASSERT_TRUE(trie_insert(t, test_strings[i],
               (void*) hash_string(test_strings[i]), NULL));
   }

   // Sanity check
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Make sure we can't remove nodes without value even if the subkey is
   // present in the tree.
   CU_ASSERT_FALSE(trie_remove(t, "tt", NULL));
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Remove a descendant and check value
   void * data = 0;
   CU_ASSERT_TRUE(trie_remove(t, "tttt", &data));
   --string_count;
   CU_ASSERT_EQUAL(data, (void*) hash_string("tttt"));

   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Remove missing one
   CU_ASSERT_FALSE(trie_remove(t, "tttt", NULL));

   // Remove remaining
   CU_ASSERT_TRUE(trie_remove(t, "asdf", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttdb", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttt", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttd", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttm", NULL));

   CU_ASSERT_EQUAL(trie_size(t), 0);

   // Check destroy is working
   trie_destroy(t, NULL);
}

static void test_remove_sebtest_two ()
{
   trie_t t = trie_new();
   CU_ASSERT_PTR_NOT_NULL(t);

   uintptr_t countdown = 0;

   // Test walker for empty tree; Should return true
   CU_ASSERT_TRUE(trie_walk(t, test_walk_walker2, &countdown));

   const char * test_strings[] = {"ttt", "ttm", "ttd", "ttdb", "hhg", "tttt", "ccc", "aa", "ee", "dd", "ff", "ax", "jello", "x", "y", "z", "xxx", "xt", "zztop", "xy"};
   unsigned int string_count =
      sizeof(test_strings)/sizeof(test_strings[0]);

   for (unsigned int i=0; i<string_count; ++i)
   {
      CU_ASSERT_TRUE(trie_insert(t, test_strings[i],
               (void*) hash_string(test_strings[i]), NULL));
   }

   // Sanity check
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Make sure we can't remove nodes without value even if the subkey is
   // present in the tree.
   CU_ASSERT_FALSE(trie_remove(t, "tt", NULL));
   CU_ASSERT_EQUAL(trie_size(t), string_count);

//   // Remove a descendant and check value
   void * data = 0;
   CU_ASSERT_TRUE(trie_remove(t, "ccc", &data));
   --string_count;
   CU_ASSERT_EQUAL(data, (void*) hash_string("ccc"));
//
   CU_ASSERT_EQUAL(trie_size(t), string_count);

   // Remove missing one
   CU_ASSERT_FALSE(trie_remove(t, "ccc", NULL));

   // Remove remaining
   CU_ASSERT_TRUE(trie_remove(t, "ee", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "hhg", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "xxx", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttt", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttm", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttd", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ttdb", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "tttt", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "aa", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "dd", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ff", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "jello", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "ax", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "x", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "y", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "z", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "xt", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "xy", NULL));
   CU_ASSERT_TRUE(trie_remove(t, "zztop", NULL));

   CU_ASSERT_EQUAL(trie_size(t), 0);

   // Check destroy is working
   trie_destroy(t, NULL);
}

static void test_remove ()
{
   for (unsigned int loop=0; loop<10; ++loop)
   {
      trie_t t = trie_new();
      CU_ASSERT_PTR_NOT_NULL(t);

      char * test_strings[10];
      const unsigned int string_count =
         sizeof(test_strings)/sizeof(test_strings[0]);

      enum { BAD_LUCK = 100000 };

      // Generate string_count random strings
      unsigned int todo = 0;
      unsigned int safetystop = BAD_LUCK;
      while (todo != string_count)
      {
         char buf[512];
         generate_random_string(buf, sizeof(buf));

         trie_pos_t pos;
         bool b = trie_insert (t, buf, (void*) hash_string(buf), &pos);
         if (!b)
         {
            if (! --safetystop)
            {
               // stop the test, something must be wrong
               CU_ASSERT_TRUE_FATAL(safetystop);
            }
            continue;
         }

         // Reset safety stop
         safetystop = BAD_LUCK;

         CU_ASSERT_EQUAL(hash_string(buf), trie_get_value(t, pos));

         // Store string for later
         test_strings[todo]=malloc (strlen(buf)+1);
         strcpy (test_strings[todo], buf);
         ++todo;
      }

      unsigned int inserted = trie_size(t);
      CU_ASSERT_EQUAL(string_count, inserted);

      // Try removing a missing string
      CU_ASSERT_FALSE(trie_remove (t, "00092131232", NULL));

//      // Remove one by one in random order
      while (todo)
      {
         unsigned int p = rand() % todo;
         const char * str = test_strings[p];

         // Remove str
         void * data = 0;

         // Remove known entry in trie
         CU_ASSERT_TRUE(trie_remove (t, str, &data));

         --todo;

         // Trie should be one smaller now
         CU_ASSERT_EQUAL(trie_size(t), todo);

         // Check if the value of the entry matches
         CU_ASSERT_EQUAL((uintptr_t) data, hash_string(str));

         // Removing a missing entry should fail
         CU_ASSERT_FALSE(trie_remove (t, str, &data));

         // Update array by copying the last element of the array
         // into the position of the recently removed element.
         // (unless the element happened to be at the end of the array anyway)
         free (test_strings[p]);
         if (p != todo)
         {
            test_strings[p]=test_strings[todo];
         }
      }

      trie_destroy(t, NULL);
   }
}


static int init_suite1 ()
{
    return 0;
}

static int clean_suite1 ()
{
    return 0;
}

int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("trie", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   // This can be used to silence an 'unused static function' warning
   // when uncommenting tests below.
   (void) test_remove;
   (void) test_remove_fixed;
   (void) test_remove_sebtest_two;
   (void) sebtest;

   // You can uncomment tests you're not ready for below
   if ((NULL == CU_add_test(pSuite, "create_destroy", test_create_destroy))
    || (NULL == CU_add_test(pSuite, "trie_set_get", test_set_get))
    || (NULL == CU_add_test(pSuite, "trie_insert", test_insert))
    || (NULL == CU_add_test(pSuite, "trie_find", test_find))
    || (NULL == CU_add_test(pSuite, "trie_size", test_size))
    || (NULL == CU_add_test(pSuite, "trie_insert_random", test_insert_random))
    || (NULL == CU_add_test(pSuite, "trie_walk", test_walk))
    || (NULL == CU_add_test(pSuite, "trie_remove_fixed", test_remove_fixed))
    || (NULL == CU_add_test(pSuite, "trie_remove_sebtest", test_remove_sebtest))
    || (NULL == CU_add_test(pSuite, "trie_remove_sebtest_two", test_remove_sebtest_two))
    || (NULL == CU_add_test(pSuite, "trie_remove", test_remove))
       )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}


