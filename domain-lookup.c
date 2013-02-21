/* domain-lookup.c is Copyright (c) 2013 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.



   this.is.how.domains.are.stored.com
   seeing.how.domains.are.stored.com
   is.very.interesting.com

             [head]
               |
              com
              /\
        stored  interesting
       /                  \
      are                 very
       /                     \
      domains                is
        /
       how
       /\
     is  seeing
     /
    this

*/


#include "domain-lookup.h"
#include <string.h>
#include <stdlib.h>

/* Single internal function for walking, defined bellow. */
static struct domain_lookup_tree* walk_dlt(struct domain_lookup_tree *head, const char *domain_immutable, int with_data);

/* The following three functions comprise the entire API. */
struct domain_lookup_tree* init_dlt()
{
	struct domain_lookup_tree *head = malloc(sizeof(struct domain_lookup_tree));
	if (!head)
		return 0;
	memset(head, 0, sizeof(struct domain_lookup_tree));
	return head;
}
void* find_dlt(struct domain_lookup_tree *head, const char *domain)
{
	/* If head is non-null, walk_dlt will always return non-null. */
	return walk_dlt(head, domain, 1)->data;
}
int insert_dlt(struct domain_lookup_tree *head, const char *domain, void *data)
{
	head = walk_dlt(head, domain, 0);
	if (!head) /* This should only fail when malloc fails. */
		return -1;
	head->data = data;
	return 0;
}

/* Where the actual meat happens. This walks down the tree starting at head.
 * If with_data is true, it returns the closest matching domain's data that's
 * been explicitly inserted. Otherwise, it returns an existing or newly-created
 * node for the provided domain. */
static struct domain_lookup_tree* walk_dlt(struct domain_lookup_tree *head, const char *domain, int with_data)
{
	struct domain_lookup_tree *child, *data_head;
	const char *component, *right_dot;
	int i, len, component_len;

	/* We immediately return the root node for null, "", and "#". */
	if (!domain)
		return head;
	len = strlen(domain);
	if (!len)
		return head;
	if (!strcmp(domain, "#"))
		return head;

	/* data_head keeps track of the closest match that has been
	 * explicitly inserted -- which amounts to a node having a non-null
	 * data member. */
	data_head = head;

	/* The right-most dot starts as the null byte at the end of the string. */
	right_dot = &domain[len];

	/* Iterate backward through the domain. */
	for (i = len - 1; i >= 0; --i) {
		/* We're only interested when we reach a dot or the first character. */
		if (domain[i] != '.' && i)
			continue;

		/* If we're at a dot, then our component is everything to the right of the dot.
		 * Note that this does not overflow, because right_dot is initialized to be
		 * domain[len], which is the max value of i + 1. */
		if (domain[i] == '.')
			component = &domain[i + 1];
		else /* Otherwise, we're at the first character of the domain, and it's not a dot, */
			component = &domain[i]; /* so our component just begins at this index. */

		/* right_dot - component is the length of the component, up to the previous right dot. */
		component_len = (int)(right_dot - component);
		
		/* We adjust right_dot to be this latest dot we've found. If domain[i] isn't a dot,
		   i == 0, so this will be the last iteration anyway. */
		right_dot = &domain[i];

		/* If the component has no length, we skip it. This automatically elides adjacent dots. */
		if (!component_len)
			continue;

		/* We iterate through all the children of the current head looking for a match. */
		for (child = head->first_child; child; child = child->next_sibling) {
			/* We want to compare only component_len bytes and also check that they have the same len. */
			if (!strncmp(component, child->component, component_len) && strlen(child->component) == component_len)
				break;
		}

		if (child) { /* child is non-null in the case where a match is found. */
			/* We update the current head and make sure to update data_head
			 * if we found a child that was explicitly added. */
			head = child;
			if (head->data)
				data_head = head;
		} else { /* child is null in the case where no matching component was found. */
			/* If we're just searching for the closest explicitly added
			 * match, return it immediately. */
			if (with_data)
				return data_head;
			/* Otherwise, create a new child node and insert it. */
			child = init_dlt();
			if (!child) /* init_dlt calls malloc, which can fail. */
				return 0;
			/* The same length calculation from before. */
			child->component = strndup(component, component_len);
			if (!child->component) /* strdup calls malloc, which can fail. */
				return 0;
			/* Insert the new child at the front of the child list for the current head. */
			child->next_sibling = head->first_child;
			head->first_child = child;
			/* Update the head to the new child. */
			head = child;
		}
	}
	/* If we made it here, we've either matched for every component, or created [a] new child[ren].
	 * In either case, we return either data_head or head, depending on which was requested. */
	if (with_data)
		return data_head;
	return head;
}
