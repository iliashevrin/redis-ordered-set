# Redis Ordered Set
This module implements an Ordered Set data structure in Redis. An Ordered Set, or an order maintenance set, allows for constant time order comparison between members of the set.

Like regular Redis Sets and Sorted Sets, members in an Ordered Set are unique. Unlike regular Set but similarly to Sorted Sets, members in an Ordered Set are ordered. Unlike Sorted Sets where order is by score and/or lexicographical, the order of members in an Ordered Set is set when they are created.

## Implementation notes

The ordered set is basically the familiar LinkedHashSet Java class - a doubly linked list with an underlying hash table that provides constant time queries of next and previous elements for any given element. Each element is also equiped with an integer label according to its order in the set.

The labeling implementation is based on [this article](http://erikdemaine.org/papers/DietzSleator_ESA2002/paper.pdf) (Two Simplified Algorithms for Maintaining Order in a List, Bender et al), and employs one level of indirection as described in section #2 of [this article](https://www.cs.cmu.edu/~sleator/papers/maintaining-order.pdf) (Dietz and Sleator, Two Algorithms for Maintaining Order in a List) enabling an amortized O(1) insertion of new elements.

Note that the mentioned papers describe a data structure that is a list (i.e. duplicates are allowed). However, owing to the fact that we map between command line strings to elements in the structure via a hash table, uniqueness must be mainted, so the structure is eventually a set.

## Quickstart

In order to use the module simply clone the repo into your machine and run make inside the directory. Then load the module into Redis by adding loadmodule /path/to/orderedset.so into redis.conf file.

Here's an example showing how to use the new data structure:

```
redis> os.addhead my_ordered_set foo
(integer) 1
redis> os.addafter my_ordered_set foo bar baz
(integer) 2
redis> os.addbefore my_ordered_set baz bar1 bar2 bar3
(integer) 3
redis> os.compare my_ordered_set baz foo
(integer) 1
redis> os.compare my_ordered_set foo baz
(integer) -1
redis> os.members my_ordered_set
 1) "foo"
 2) "bar"
 3) "bar1"
 4) "bar2"
 5) "bar3"
 6) "baz"

```

## Commands

### OM.ADDHEAD key member [member ...]

> Time complexity: O(k), where k is the number of added members

Adds one or more `member` to the head of an ordered set stored in `key`.

#### A note about complexity

All `OM.ADD*` commands require labeling, hence when adding a single member they demonstrate O(1) amortized time complexity, and O(logN) worst case time where N is the ordered set's cardinality.

#### More about addition in general

If one or more 'member' is already found in the set, the commands updates their position. If two or more 'member' are supplied, their order after the addition will resemble the order of the passed arguments, i.e.

```
redis> os.addhead my_ordered_set a
(integer) 1
redis> os.addafter my_ordered_set b c d
(integer) 3
redis> os.members my_ordered_set
 1) "b"
 2) "c"
 3) "d"
 4) "a"

```

#### Return value

Integer reply: the number of members added to the ordered set.

### OM.ADDTAIL key member [member ...]

> Time complexity: O(k), where k is the number of added members

Adds one or more `member` to the tail of an ordered set stored in `key`.

#### Return value

Integer reply: the number of members added to the ordered set.

### OM.ADDAFTER key existing member [member ...]

> Time complexity: O(k), where k is the number of added members

Inserts one or more `member` immediately after the `existing` member in an ordered set stored in `key`.

#### Return value

Integer reply: the number of members added to the ordered set, or `nil` if `key` does not exist or `existing` is not found in it.

### OM.ADDBEFORE key existing member [member ...]

> Time complexity: O(k), where k is the number of added members

Inserts one or more `member` immediately before the `existing` member in an ordered set stored in `key`.

##### Return value

Integer reply: the number of members added to the ordered set, or `nil` if `key` does not exist or `existing` is not found in it.

### OM.REM key member [member ...]

> Time complexity: O(k), where k is the number of removed members

Removes one or more `member` from an ordered set stored in `key`.

#### Return value

Integer reply: the number of members removed from the ordered set, or `nil` if `key` does not exist.

### OM.REMHEAD key [count]

> Time complexity: O(k), where k is the number of removed members

Removes up to `count` members from the head of the ordered set stored in `key` .

`count` may be any positive non-zero integer. When absent, the single head member is removed. This operation will remove less than `count` members if the tail is encountered prematurely.

#### Return value

Integer reply: the number of members removed from the ordered set, or `nil` if `key` does not exist.

### OM.REMTAIL key [count]

> Time complexity: O(k), where k is the number of removed members

Removes up to `count` members from the tail of the ordered set stored in `key` .

`count` may be any positive non-zero integer. When absent, the single tail member is removed. This operation will remove less than `count` members if the head is encountered prematurely.

#### Return value

Integer reply: the number of members removed from the ordered set, or `nil` if `key` does not exist.

### OM.COMPARE key member1 member2

> Time complexity: O(1)

Compares the position of `member1` and `member2` in an ordered set stored in `key`.

#### Return value

Integer reply: a number represeting the comparison's result, specifically:

* **-1** when `member1` is before `member2`
* **0** when `member1` is identical to `member2` <- i.e. `om.compare k foo foo`
* **1** when `member1` is after `member2`
* **nil** when `key` does not exist, and when `member1` or `member2` are not found in it

### OM.MEMBERS key

> Time complexity: O(N), where N is the ordered set's cardinality

Returns all members from head to tail in the ordered set stored in `key`.

#### Return value

Array reply: all members of the ordered set.

### OM.NEXT key member [count]

> Time complexity: O(k), where k is the returned members count

Returns up to `count` members that follow `member` in the ordered set stored in `key` .

`count` may be any positive integer, and when set to **0** all members until the tail are returned. When absent, the immediate next member is returned. This operation will return less than `count` members if the tail is encountered prematurely.

#### Return value

Array reply: the members in the ordered set.

### OM.PREV key member [count]

> Time complexity: O(k), where k is the returned members count

Returns in reverse order up to  `count` members that precede `member` in the ordered set stored in `key` .

`count` may be any positive integer, and when set to **0** all members up to the head are returned. When absent, the immediate previous member is returned. This operation will return less than `count` members if the head is encountered prematurely.

#### Return value

Array reply: the members in the ordered set in reverse order.

### OM.HEAD key [count]

> Time complexity: O(k), where k is the returned members count

Returns up to `count` members from the head of the ordered set stored in `key` .

`count` may be any positive integer, and when set to **0** all members until the tail are returned. When absent, the single head member is returned. This operation will return less than `count` members if the tail is encountered prematurely.

#### Return value

Array reply: the members in the ordered set.

### OM.TAIL key [count]

> Time complexity: O(k), where k is the returned members count

Returns in reverse order up to `count` members from the tail of the ordered set stored in `key` .

`count` may be any positive integer, and when set to **0** all members until the head are returned. When absent, the single tail member is returned. This operation will return less than `count` members if the head is encountered prematurely.

#### Return value

Array reply: the members in the ordered set in reverse order.

### OM.CARD key

> Time complexity: O(1)

Returns the cardinality (number of members) of the ordered set stored in `key` .

#### Return value

Integer reply: the cardinality (number of elements) of the set, or `0` if `key` does not exist
