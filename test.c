#include "domain-lookup.h"
#include <stdio.h>

#define print_lookup(head, domain) printf("%s: %s\n", domain, (char *)find_dlt(head, domain))

void main()
{
	struct domain_lookup_tree *head = init_dlt();
	insert_dlt(head, "zx2c4.com", "zx2c4 root node");
	insert_dlt(head, ".data.zx2c4.com", "zx2c4 data node");
	insert_dlt(head, "co.uk.", "co.uk root node");
	insert_dlt(head, "yahoo.co.uk", "yahoo.co.uk node");
	insert_dlt(head, "yahoo.co.uk", "yahoo.co.uk node overwrite");
	insert_dlt(head, "something.co.uk", "something town, london");
	insert_dlt(head, "yonder.co.uk", "yonder town, manchester");
	insert_dlt(head, "#", "null node");

	print_lookup(head, "blog.zx2c4.com");
	print_lookup(head, "data.zx2c4.com");
	print_lookup(head, "dat.zx2c4.com");
	print_lookup(head, "zx2c4.com");
	print_lookup(head, "blala.asdf.adsf.adsf.data.zx2c4.com");
	print_lookup(head, "");
	print_lookup(head, "yahoo.com.");
	print_lookup(head, ".news.yahoo.co.uk.");
	print_lookup(head, "british.co.uk");
	print_lookup(head, "other.com");
	print_lookup(head, "test.jj.uk");
	print_lookup(head, "tasdf.asdf.jj.uk.");
	print_lookup(head, "yahoo.co.uk.bad");
	print_lookup(head, "..yahoo.co.uk");
	print_lookup(head, ".ahaan....data...zx2c4.com");
	print_lookup(head, "data..zx2c4.com...");
	print_lookup(head, "asdf.yonder.co.uk");
	print_lookup(head, "ananana.something.co.uk.");
}
