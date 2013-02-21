struct domain_lookup_tree {
	char *component;
	void *data;
	struct domain_lookup_tree *first_child;
	struct domain_lookup_tree *next_sibling;
};

struct domain_lookup_tree* init_dlt();
void* find_dlt(struct domain_lookup_tree *head, const char *domain);
int insert_dlt(struct domain_lookup_tree *head, const char *domain, void *data);
