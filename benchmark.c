#include "domain-lookup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static unsigned long range_rand(unsigned long min, unsigned long max)
{
	unsigned long base_random, range, remainder, bucket;

	base_random = rand();
	if (RAND_MAX == base_random)
		return range_rand(min, max);
	range = max - min;
	remainder = RAND_MAX % range;
	bucket = RAND_MAX / range;
	if (base_random < RAND_MAX - remainder)
		return min + base_random / bucket;
	return range_rand(min, max);
}


static char* make_domain(char **dictionary, size_t size, size_t max_components)
{
	char *domain;
	int i;
	
	domain = malloc(512);
	domain[0] = 0;

	/* strcat can overflow. I don't care. */
	for (i = 0; i < range_rand(2, max_components + 1); ++i) {
		if (i > 0)
			strcat(domain, ".");
		strcat(domain, dictionary[range_rand(0, size)]);
	}

	return domain;
}


static void* old_find_dlt(char *domain, char **domains, size_t domains_size)
{
	unsigned int namelen, matchlen, domainlen, i;
	char *matchstart, *out;

	namelen = strlen(domain);
	matchlen = 0;
	out = 0;
	for (i = 0; i < domains_size; ++i) {
		domainlen = strlen(domains[i]);
		matchstart = domain + namelen - domainlen;
		if (namelen >= domainlen &&
		   (domainlen == 0 || namelen == domainlen || *(matchstart - 1) == '.' ) &&
		    domainlen >= matchlen && !strcmp(matchstart, domains[i]))  {
			matchlen = domainlen;
			out = domains[i];
		}
	}
	return out;
}


#define DICTIONARY_INITIAL_SIZE		250000
#define DICTIONARY_TRUNCATED_SIZE	300
#define INSERTED_DOMAINS_SIZE		100000
#define QUERIED_DOMAINS_SIZE		100000
#define INSERTED_DOMAINS_COMPONENTS	3
#define QUERIED_DOMAINS_COMPONENTS	5
#define DICTIONARY_PATH			"/usr/share/dict/words"

int main()
{
	FILE *dictionary_file;
	char **dictionary, **inserted_domains, **queried_domains;
	void **verification_new, **verification_old;
	char *word;
	size_t dictionary_size, word_size, i;
	struct domain_lookup_tree *head;
	clock_t start, old_time, new_time;

	srand(time(NULL) + 42);

	printf("[+] Populating in-memory word list from " DICTIONARY_PATH ".\n");

	dictionary = malloc(DICTIONARY_INITIAL_SIZE * sizeof(char *));
	dictionary_file = fopen(DICTIONARY_PATH, "r");
	if (!dictionary_file || !dictionary)
		return 1;
	for (dictionary_size = 0; dictionary_size < DICTIONARY_INITIAL_SIZE; ++dictionary_size) {
		word_size = 256;
		word = malloc(word_size);
		if (!word)
			return 1;
		if (getline(&word, &word_size, dictionary_file) < 0)
			break;
		word_size = strlen(word);
		if (word[word_size - 1] == '\n')
			word[word_size - 1] = 0;
		dictionary[dictionary_size] = word;
	}
	fclose(dictionary_file);

	printf("[+] Truncating in-memory word list to %d random words.\n", DICTIONARY_TRUNCATED_SIZE);

	for (i = 0; i < DICTIONARY_TRUNCATED_SIZE; ++i)
		dictionary[i] = dictionary[range_rand(0, dictionary_size)];

	printf("[+] Creating random lists of %d domains to insert and %d domains to query.\n", INSERTED_DOMAINS_SIZE, QUERIED_DOMAINS_SIZE);

	inserted_domains = malloc(INSERTED_DOMAINS_SIZE * sizeof(char *));
	queried_domains = malloc(QUERIED_DOMAINS_SIZE * sizeof(char *));
	verification_new = malloc(QUERIED_DOMAINS_SIZE * sizeof(char *));
	verification_old = malloc(QUERIED_DOMAINS_SIZE * sizeof(char *));
	if (!inserted_domains || !queried_domains || !verification_new || !verification_old)
		return 1;

	for (i = 0; i < QUERIED_DOMAINS_SIZE; ++i)
		queried_domains[i] = make_domain(dictionary, DICTIONARY_TRUNCATED_SIZE, QUERIED_DOMAINS_COMPONENTS);
	for (i = 0; i < INSERTED_DOMAINS_SIZE; ++i)
		inserted_domains[i] = make_domain(dictionary, DICTIONARY_TRUNCATED_SIZE, INSERTED_DOMAINS_COMPONENTS);

	printf("[+] Populating domain lookup tree.\n");

	head = init_dlt();
	for (i = 0; i < INSERTED_DOMAINS_SIZE; ++i)
		insert_dlt(head, inserted_domains[i], inserted_domains[i]);

	printf("[+] Performing lookup benchmarks:\n");

	start = clock();
	for (i = 0; i < QUERIED_DOMAINS_SIZE; ++i)
		verification_new[i] = find_dlt(head, queried_domains[i]);
	new_time = clock() - start;
	printf("    [*] New method took %.2f seconds.\n", (double)new_time / CLOCKS_PER_SEC);

	start = clock();
	for (i = 0; i < QUERIED_DOMAINS_SIZE; ++i)
		verification_old[i] = old_find_dlt(queried_domains[i], inserted_domains, INSERTED_DOMAINS_SIZE);
	old_time = clock() - start;
	printf("    [*] Old method took %.2f seconds.\n", (double)old_time / CLOCKS_PER_SEC);

	printf("    [*] The new method is %f times faster than the old method.\n", (double)old_time / (double)new_time);

	printf("[+] Verifying that new and old methods produced identical results:\n");
	for (i = 0; i < QUERIED_DOMAINS_SIZE; ++i) {
		if (verification_old[i] != verification_new[i]) {
			printf("    [!] New and old method produced different results!\n");
			return 1;
		}
	}
	printf("    [*] New and old methods produced the same results.\n");
	return 0;
}
