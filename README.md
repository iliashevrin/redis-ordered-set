# Redis Ordered Set
Order Maintenance Set for Redis

This module provides an order maintenance set in Redis, that allows for O(1) order comparison between elements in the set. The implementation based on <a href="http://erikdemaine.org/papers/DietzSleator_ESA2002/paper.pdf">this article</a>, with one level of indirection described in <a href="https://www.cs.cmu.edu/~sleator/papers/maintaining-order.pdf">this article</a>. The indirection allows for amortized O(1) insertion of new elements. The implementation uses an underlying hash table that provides constant time queries of next and previous elements for any given element. Note that these papers describe a data structure which is a list (meaning that duplicates are allowed). However, owing to the fact that we map between command line strings to elements in the structure via a hash table, uniqueness must be mainted, so the structure is eventually a set.

## Quickstart

In order to use the module simply clone the repo into your machine and run make inside the directory. Then load the module into Redis by adding loadmodule /path/to/orderedset.so into redis.conf file.

## Commands

#### OM.ADDFIRST key x

Inserts x into head of ordered set with key name. Creates structure if does not exist under given key name. All 'add' commands require labeling, hence addfirst runs O(1) amortized and O(logN) worst case time when N is the number of elements in the set.

#### OM.ADDLAST key x

Inserts x into tail of ordered set with key name. Creates structure if does not exist under given key name. All 'add' commands require labeling, hence addlast runs O(1) amortized and O(logN) worst case time when N is the number of elements in the set.

#### OM.ADDAFTER key x y

Inserts y right after x in ordered set with key name. Returns ERR if key does not exist or if x is not found. All 'add' commands require labeling, hence addafter runs O(1) amortized and O(logN) worst case time when N is the number of elements in the set.

#### OM.ADDBEFORE key x y

Inserts y right before x in ordered set with key name. Returns ERR if key does not exist or if x is not found. All 'add' commands require labeling, hence addbefore runs O(1) amortized and O(logN) worst case time when N is the number of elements in the set.

#### OM.REMOVE key x

Removes x from ordered set with key name. Deletes the structure if x was the last element to remove. Returns ERR if key does not exist or if x is not found. Runs O(1) worst case time.

#### OM.COMPARE key x y

Compares x and y based on their position in the set with key name. Returns -1 if x is before y, 0 if they are the same, and 1 if x is after y. Returns ERR if key does not exist or if x is not found. Runs O(1) worst case time.

#### OM.LIST key

Lists all elements in the set with key name from head to tail. Returns ERR if key does not exist. Runs O(N) when N is the number of elements in the set.

#### OM.NEXT key x k

Lists the next k elements following x in the set with key name. Returns less than k elements if reached the tail of the set. Returns ERR if key does not exist or if x is not found. All query commands take O(k) time when k is the number of elements in the output.

#### OM.PREV key x k

Lists the previous k elements behind x in reverse order in the set with key name. Returns less than k elements if reached the head of the set. Returns ERR if key does not exist or if x is not found. All query commands take O(k) time when k is the number of elements in the output.

#### OM.FIRST key k

Lists the first k elements in the set with key name. Returns less than k elements if reached the tail of the set. Returns ERR if key does not exist. All query commands take O(k) time when k is the number of elements in the output.

#### OM.LAST key k

Lists the last k elements in reverse order in the set with key name. Returns less than k elements if reached the head of the set. Returns ERR if key does not exist. All query commands take O(k) time when k is the number of elements in the output.

#### OM.SIZE key

Returns the number of elements in the set with key name. Returns ERR if key does not exist. Runs in O(1) worst case time.
