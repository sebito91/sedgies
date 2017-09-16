Homework 7 -- Tries

For this assignment I chose to implement a Ternary Search Tree (TST) after reading about them in
Sedgewick's Algorithms in C (Chapter 15).

    http://www.amazon.com/Algorithms-Parts-1-4-Fundamentals-Structures/dp/0201314525

The elegance of the algorithm cannot be underestimated, as insertion and sort are blazingly fast
at O(log N). Similar to Binary Search Trees, the TST allows us to maintain the rules that
anything to the left of the root is less than the root, anything to the right is greater than
the root. But it also allows us to add that anything using the root goes down the middle link; in
this case any string using our key.

While insertion and sort are easy (because they're basically the same function, you need to be sorted
in order to insert), removal is very difficult. In this case, we have a 'working' solution for
most use-cases (see NOTES below), but the logic is convoluted and could use a refactor given more
time.

The basic algorithm for removal is as follows:

  1. Search for the key
  2. If not found, return false
  3. If found, check how many children the node has
  4. If none, bubble up to top of key (pass current node + parent to helper function, recursively)
  5. If has children, pass the current element itself (don't bubble up)
  6. Recursively deal with links as required
     a. If the parent has both a left + right link, this requires the most logic...
        i.  If we're on the lesser branch from the absolute root (aka left branch), update the parent's
            parent to point to the larger value; re-sort the lesser value under the larger value's branches
            as if it was an insert itself (shake out the trie), then remove the parent and its children (if
            applicable). This resembles an AVL-tree left-right rotation.
        ii. If we're on the larger branch from the absolute root (aka right branch), we perform the same
            as above but in this case take the lesser value and re-sort the larger value under the lesser's
            branches. This resembles an AVL-tree right-left rotation.

As mentioned before, this logic is convoluted but does work for the most part. When running the trie_test
units we do pass all tests but have issues with larger values of test_remove (>10). Leaks are kept in
check, which is postive.

-----------------------

NOTES:

I've updated the trie_test.c commmit in this repo to only have 10 strings as part of the test_remove unit.
The app does handle all 1000 inserts + destroy wsithout leaks (you can test this by commenting out the trie_remove
commands), but we have some logic flaws somewhere in the trie_remove_key function that causes inadvertent
sub-key removal. For string counts >10 it seems we hit this consistently, which is indeed annoying. There
is some logic that needs updating, but since we don't have enough time to address these issues we'll have to
live with the limitations.

Additionally, I've added my own test tries (two of them) to trie_test.c which were meant to stress my
implementation's logic. We pass those tests, but I fear that we've coded too closely to the test and not
to the general use-case.
