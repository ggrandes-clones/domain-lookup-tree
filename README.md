# Domain Lookup Tree
### by Jason A. Donenfeld (<Jason@zx2c4.com>)

Some applications, like [dnsmasq](http://www.thekelleys.org.uk/dnsmasq/doc.html), need to find the closest-match domain name. One way is by iterating through a list of domains and finding the one that has the longest length match. But a more efficient way is to build a tree of domain name components.

This project provides a very simple C API for a domain name matching tree. The **[source code is simple, extremely well commented, and easy to read](http://git.zx2c4.com/domain-lookup-tree/tree/domain-lookup.c)**.

## API

#### `struct domain_lookup_tree* init_dlt()`

Constructs a new tree head and returns a pointer to it.

#### `void* find_dlt(struct domain_lookup_tree *head, const char *domain)`

Returns the prior inserted data in `head` for a given `domain`.

#### `int insert_dlt(struct domain_lookup_tree *head, const char *domain, void *data)`

Inserts `data` into a tree given by `head` for a given `domain`. If `domain` has length of zero or is equal to `#`, it will act as the default match for non-matching lookups.


## Example

    struct domain_lookup_tree *domains = init_dlt();

    insert_dlt(domains, "zx2c4.com", "zx2c4 root node");
    insert_dlt(domains, "data.zx2c4.com", "zx2c4 data node");
    insert_dlt(domains, "yahoo.co.uk", "yahoo.co.uk node");
    insert_dlt(domains, "#", "generic node");

    char *data = find_dlt(domains, "cheese.somewhere.zx2c4.com");
    printf("zx2c4 root node == %s\n", data);

## Benchmarks

With gcc 4.7.2 on an Intel Core i7-3820QM, we show a **~587x speed improvement** over dnsmasq's old algorithm:

    zx2c4@thinkpad ~/Projects/domain-lookup-tree $ make
    cc -march=native -pipe -fomit-frame-pointer -flto -O3    benchmark.c domain-lookup.c domain-lookup.h   -o benchmark

    zx2c4@thinkpad ~/Projects/domain-lookup-tree $ ./benchmark 
    [+] Populating in-memory word list from /usr/share/dict/words.
    [+] Truncating in-memory word list to 300 random words.
    [+] Creating random lists of 100000 domains to insert and 100000 domains to query.
    [+] Populating domain lookup tree.
    [+] Performing lookup benchmarks:
        [*] New method took 0.34 seconds.
        [*] Old method took 199.45 seconds.
        [*] The new method is 586.617647 times faster than the old method.
    [+] Verifying that new and old methods produced identical results:
        [*] New and old methods produced the same results.

## Limitations & Improvements

* Currently no helper functions for removing and freeing nodes
* Instead of `strcmp`'ing, a hash function could be used
