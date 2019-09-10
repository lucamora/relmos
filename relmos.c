#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100
#define RED 'r'
#define BLACK 'b'

typedef struct node
{
	void *data;
	char color;
	struct node *left, *right, *parent;
} node_t;

typedef struct max
{
	char *name;
	struct max *next;
} max_t;

typedef struct typ
{
	char *name;
	int count;
	max_t *dsts;
	struct typ *next;
} type_t;

typedef struct rel
{
	char *type;
	struct rel *next;
} rel_t;

typedef struct el
{
	char *src;
	rel_t *types;
} src_t;

typedef struct cnt
{
	int count;
	type_t *type;
	struct cnt *next;
} count_t;

typedef struct
{
	char *name;
	count_t *counts;
	node_t *sources;
} ent_t;


void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();
void end();


// private helper function
type_t *_addreltype(char *name);
type_t *_getreltype(char *name);
void _delsrc(char *name, type_t *rel, ent_t *curr_dst);
void _delsources(node_t *root, char *ent);
void _increment(ent_t *dst, type_t *type);
void _decrement(ent_t *dst, char *type);
char _buildreport(node_t *root);


// tree operations
void tree_left_rotate(node_t **root, node_t *x);
void tree_right_rotate(node_t **root, node_t *x);
void tree_insert_fixup(node_t **root, node_t *z);
void tree_delete_fixup(node_t **root, node_t *x);
node_t *tree_minimum(node_t *x);
node_t *tree_successor(node_t *x);

node_t *entities_search(node_t *node, char *ent);
void entities_insert(node_t **root, char *ent);
void entities_delete(node_t **root, node_t *z);
void entities_dispose(node_t *x);

node_t *sources_search(node_t *node, char *ent);
src_t *sources_insert(node_t **root, char *ent);
void sources_delete(node_t **root, node_t *z);
void sources_dispose(node_t *x);

#define NIL (&nil)
static node_t nil = { NULL, BLACK, &nil, &nil, &nil };


// main data structures
node_t *destinations = NIL;
type_t *types = NULL;


int main(int argc, char const *argv[])
{
	char cmd[7];
	size_t len = 0;
	int read = 0;
	char *src = NULL, *dst = NULL, *rel = NULL;

	do
	{
		scanf("%s%*c", cmd);
		if (cmd[0] == 'a')
		{
			// addent or addrel
			if (cmd[3] == 'e')
			{
				// addent
				src = NULL;
				len = 0;
				read = getline(&src, &len, stdin);
				src[read - 1] = '\0';
				//printf("addent: entity: '%s' (%d)\n", src, read);
				addent(src);
			}
			else
			{
				// addrel
				src = malloc(MAX_INPUT);
				dst = malloc(MAX_INPUT);
				rel = malloc(MAX_INPUT);
				scanf("%s%*c%s%*c%s", src, dst, rel);

				//printf("addrel: src: '%s', dst: '%s', relation: '%s'\n", src, dst, rel);
				addrel(src, dst, rel);
			}
		}
		else if (cmd[0] == 'd')
		{
			// delent or delrel
			if (cmd[3] == 'e')
			{
				// delent
				src = NULL;
				len = 0;
				read = getline(&src, &len, stdin);
				src[read - 1] = '\0';
				//printf("delent: entity: '%s'\n", src);
				delent(src);
			}
			else
			{
				// delrel
				src = malloc(MAX_INPUT);
				dst = malloc(MAX_INPUT);
				rel = malloc(MAX_INPUT);
				scanf("%s%*c%s%*c%s", src, dst, rel);

				//printf("delrel: src: '%s', dst: '%s', relation: '%s'\n", src, dst, rel);
				delrel(src, dst, rel);
			}
		}
		else if (cmd[0] == 'r')
		{
			// report
			//printf("report\n");
			report();
		}
		else if (cmd[0] == 'e')
		{
			// end
			//printf("fine\n");
			end();
			return 0;
		}
	} while (1);
}

void addent(char *entity)
{
	entities_insert(&destinations, entity);
}

void delent(char *entity)
{
	// lookup entity
	node_t *curr_ent = entities_search(destinations, entity);

	free(entity);

	// entity does not exist
	if (curr_ent == NULL)
		return;

	// delete all occurrencies in other sources
	_delsources(destinations, ((ent_t *)curr_ent->data)->name);

	// delete entity
	entities_delete(&destinations, curr_ent);
}

void addrel(char *src, char *dst, char *rel)
{
	// search if relation type already exists
	type_t *curr_type = _addreltype(rel);

	// lookup entities
	node_t *src_ent = entities_search(destinations, src);
	node_t *dst_ent = entities_search(destinations, dst);

	free(src);
	free(dst);

	// an entity does not exist
	if (src_ent == NULL || dst_ent == NULL)
		return;

	// create source (if does not already exist)
	src_t *curr_src = sources_insert(&((ent_t *)dst_ent->data)->sources, ((ent_t *)src_ent->data)->name);

	// lookup relation
	rel_t *iter = curr_src->types;
	while (iter != NULL)
	{
		// relation already exists
		if (curr_type->name == iter->type)
			return;
		iter = iter->next;
	}
	
	// create relation	
	rel_t *new_rel = malloc(sizeof(rel_t));
	new_rel->type = curr_type->name;
	new_rel->next = curr_src->types;
	curr_src->types = new_rel;
	_increment((ent_t *)dst_ent->data, curr_type);
}

void delrel(char *src, char *dst, char *rel)
{
	// lookup relation type
	type_t *relation = _getreltype(rel);
	free(rel);

	// lookup entities
	node_t *src_ent = entities_search(destinations, src);
	node_t *dst_ent = entities_search(destinations, dst);

	free(src);
	free(dst);

	// relation or an entity does not exist
	if (relation == NULL || src_ent == NULL || dst_ent == NULL)
		return;
	
	// delete source
	_delsrc(((ent_t *)src_ent->data)->name, relation, (ent_t *)dst_ent->data);
}

void report()
{
	// number of items added to the report
	//int added = 0;
	char empty = 1;
	if (types != NULL)
	{
		empty = _buildreport(destinations);
	}

	if (empty == 0)
	{
		char first = 1;
		type_t *t = types;
		while (t != NULL)
		{
			if (t->count > 0)
			{
				if (first == 0)
					printf(" ");
				first = 0;
				printf("%s ", t->name);

				max_t *m = t->dsts;
				while (t->dsts != NULL)
				{
					printf("%s ", t->dsts->name);

					m = t->dsts;
					t->dsts = m->next;
					free(m);
				}

				printf("%d;", t->count);
				t->count = 0;
			}

			t = t->next;
		}
	}
	else
	{
		printf("none");
	}

	printf("\n");
}

void end()
{
	// free destinations
	entities_dispose(destinations);

	// free types
	type_t *type;
	while (types != NULL)
	{
		type = types;
		types = type->next;
		max_t *i = type->dsts;
		while (type->dsts != NULL)
		{
			i = type->dsts;
			type->dsts = i->next;
			free(i);
		}
		free(type->name);
		free(type);
	}
}


// private helper function
type_t *_addreltype(char *name)
{
	type_t *curr_type = types;
	type_t *ins_type = NULL;
	while (curr_type != NULL)
	{
		int cmp = strcmp(name, curr_type->name);
		if (cmp == 0)
		{
			free(name);
			return curr_type;
		}
		else
		{
			if (cmp > 0)
			{
				ins_type = curr_type;
			}
			curr_type = curr_type->next;
		}
	}

	// create relation type
	type_t *new_type = malloc(sizeof(type_t));
	new_type->name = name;
	new_type->count = 0;
	new_type->dsts = NULL;
	if (ins_type == NULL)
	{
		new_type->next = types;
		types = new_type;
	}
	else
	{
		new_type->next = ins_type->next;
		ins_type->next = new_type;
	}

	return new_type;
}

type_t *_getreltype(char *name)
{
	type_t *curr = types;
	while (curr != NULL)
	{
		if (strcmp(name, curr->name) == 0)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

void _delsrc(char *name, type_t *rel, ent_t *curr_dst)
{
	// if rel != NULL
	// then delete only specific relation
	// else delete all relations
	node_t *curr_src = sources_search(curr_dst->sources, name);

	if (curr_src == NULL)
		return;

	if (rel != NULL)
	{
		// delete only a relation
		rel_t *iter = ((src_t *)curr_src->data)->types;
		rel_t *p = NULL;
		while (iter != NULL)
		{
			if (rel->name == iter->type)
			{
				// remove relation
				if (p == NULL)
				{
					((src_t *)curr_src->data)->types = iter->next;
				}
				else
				{
					p->next = iter->next;
				}

				_decrement(curr_dst, iter->type);
				free(iter);
				iter = NULL; // exit loop
			}
			else
			{
				p = iter;
				iter = iter->next;	
			}
		}

		// delete source if it does not have any relation
		if (((src_t *)curr_src->data)->types == NULL)
		{
			sources_delete(&curr_dst->sources, curr_src);
			return;
		}
	}
	else
	{
		// delete all relations
		rel_t *r;
		while (((src_t *)curr_src->data)->types != NULL)
		{
			_decrement(curr_dst, ((src_t *)curr_src->data)->types->type);
			r = ((src_t *)curr_src->data)->types;
			((src_t *)curr_src->data)->types = r->next;
			free(r);
		}

		// delete source
		sources_delete(&curr_dst->sources, curr_src);
	}
}

void _delsources(node_t *root, char *ent)
{
	char leftdone = 0;
	char end = 0;

	while (root != NIL && end == 0)
	{
		if (leftdone == 0)
		{
			while (root->left != NIL)
				root = root->left;
		}

		if (((ent_t *)root->data)->name != ent)
			_delsrc(ent, NULL, (ent_t *)root->data);
		
		leftdone = 1;

		if (root->right != NIL)
		{
			leftdone = 0;
			root = root->right;
		}
		else if (root->parent != NIL)
		{
			while (root->parent != NIL && root == root->parent->right)
				root = root->parent;
			if (root->parent == NIL)
				end = 1;
			else
				root = root->parent;
		}
		else
			end = 1;
		
	}
}

void _increment(ent_t *dst, type_t *type)
{
	count_t *curr = dst->counts;
	while (curr != NULL)
	{
		if (type == curr->type)
		{
			curr->count++;
			return;
		}
		curr = curr->next;
	}

	curr = malloc(sizeof(count_t));
	curr->count = 1;
	curr->type = type;
	curr->next = dst->counts;
	dst->counts = curr;
}

void _decrement(ent_t *dst, char *type)
{
	count_t *curr = dst->counts;
	while (curr != NULL)
	{
		if (type == curr->type->name)
		{
			curr->count--;
			return;
		}
		curr = curr->next;
	}
}

char _buildreport(node_t *root)
{
	char leftdone = 0;
	char end = 0;
	char empty = 1;

	while (root != NIL && end == 0)
	{
		if (leftdone == 0)
		{
			while (root->left != NIL)
				root = root->left;
		}

		// start build report
		count_t *count = ((ent_t *)root->data)->counts;
		// iterate through destination counts
		while (count != NULL)
		{
			type_t *iter = types;
			char done = 0;
			// iterate through types
			while (done == 0)
			{
				if (iter == count->type)
				{
					// if current count is bigger
					// delete all dsts and add the current dst
					if (count->count > iter->count)
					{
						iter->count = count->count;

						max_t *c;
						while (iter->dsts != NULL)
						{
							c = iter->dsts;
							iter->dsts = c->next;
							free(c);
						}

						iter->dsts = malloc(sizeof(max_t));
						iter->dsts->next = NULL;
						iter->dsts->name = ((ent_t *)root->data)->name;
						empty = 0;
					}
					// if current count is equal
					// add the current dst
					else if (count->count > 0 && count->count == iter->count)
					{
						max_t *new = malloc(sizeof(max_t));
						new->name = ((ent_t *)root->data)->name;
						new->next = iter->dsts;
						iter->dsts = new;
					}

					done = 1;
				}

				iter = iter->next;
			}

			count = count->next;
		}
		// end build report
		
		leftdone = 1;

		if (root->right != NIL)
		{
			leftdone = 0;
			root = root->right;
		}
		else if (root->parent != NIL)
		{
			while (root->parent != NIL && root == root->parent->right)
				root = root->parent;
			if (root->parent == NIL)
				end = 1;
			else
				root = root->parent;
		}
		else
			end = 1;
		
	}

	return empty;
}


// generic tree operations
void tree_left_rotate(node_t **root, node_t *x)
{
	node_t *y = x->right;
	x->right = y->left;
	if (y->left != NIL)
		y->left->parent = x;
	if (y != NIL)
		y->parent = x->parent;
	if (x->parent == NIL)
		(*root) = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->left = x;
	if (x != NIL)
		x->parent = y;
}

void tree_right_rotate(node_t **root, node_t *x)
{
	node_t *y = x->left;
	x->left = y->right;
	if (y->right != NIL)
		y->right->parent = x;
	if (y != NIL)
		y->parent = x->parent;
	if (x->parent == NIL)
		(*root) = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->right = x;
	if (x != NIL)
		x->parent = y;
}

void tree_insert_fixup(node_t **root, node_t *z)
{
	while (z != (*root) && z->parent->color == RED)
	{
		node_t *x = z->parent;
		if (x->color == RED)
		{
			if (x == x->parent->left)
			{
				node_t *y = x->parent->right;
				if (y->color == RED)
				{
					x->color = BLACK;
					y->color = BLACK;
					x->parent->color = RED;
					z = x->parent;
				}
				else
				{
					if (z == x->right)
					{
						z = x;
						tree_left_rotate(root, z);
						x = z->parent;
					}
					x->color = BLACK;
					x->parent->color = RED;
					tree_right_rotate(root, x->parent);
				}
			}
			else
			{
				node_t *y = x->parent->left;
				if (y->color == RED)
				{
					x->color = BLACK;
					y->color = BLACK;
					x->parent->color = RED;
					z = x->parent;
				}
				else
				{
					if (z == x->left)
					{
						z = x;
						tree_right_rotate(root, z);
						x = z->parent;
					}
					x->color = BLACK;
					x->parent->color = RED;
					tree_left_rotate(root, x->parent);
				}
			}
		}
	}
	(*root)->color = BLACK;
}

void tree_delete_fixup(node_t **root, node_t *x)
{
	while (x != (*root) && x->color == BLACK)
	{
		if (x == x->parent->left)
		{
			node_t *w = x->parent->right;
			if (w->color == RED)
			{
				w->color = BLACK;
				x->parent->color = RED;
				tree_left_rotate(root, x->parent);
				w = x->parent->right;
			}
			if (w->left->color == BLACK && w->right->color == BLACK)
			{
				w->color = RED;
				x = x->parent;
			}
			else
			{
				if (w->right->color == BLACK)
				{
					w->left->color = BLACK;
					w->color = RED;
					tree_right_rotate(root, w);
					w = x->parent->right;
				}
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				tree_left_rotate(root, x->parent);
				x = (*root); // exit loop
			}
		}
		else
		{
			node_t *w = x->parent->left;
			if (w->color == RED)
			{
				w->color = BLACK;
				x->parent->color = RED;
				tree_right_rotate(root, x->parent);
				w = x->parent->left;
			}
			if (w->right->color == BLACK && w->left->color == BLACK)
			{
				w->color = RED;
				x = x->parent;
			}
			else
			{
				if (w->left->color == BLACK)
				{
					w->right->color = BLACK;
					w->color = RED;
					tree_left_rotate(root, w);
					w = x->parent->left;
				}
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				tree_right_rotate(root, x->parent);
				x = (*root); // exit loop
			}
		}
	}
	x->color = BLACK;
}

node_t *tree_minimum(node_t *x)
{
	while (x->left != NIL)
		x = x->left;
	return x;
}

node_t *tree_successor(node_t *x)
{
	if (x->right != NIL)
		return tree_minimum(x->right);
	node_t *y = x->parent;
	while (y != NIL && x == y->right)
	{
		x = y;
		y = y->parent;
	}
	return y;
}


// destinations tree operations
node_t *entities_search(node_t *node, char *ent)
{
	node_t *curr = node;
	while (curr != NIL)
	{
		int cmp = strcmp((((ent_t *)curr->data)->name), ent);
		if (cmp == 0)
			return curr;
		if (cmp < 0)
			curr = curr->left;
		else
			curr = curr->right;
	}
	return NULL;
}

void entities_insert(node_t **root, char *ent)
{
	node_t *y = NIL;
	node_t *x = (*root);
	while (x != NIL)
	{
		y = x;
		int cmp = strcmp(((ent_t *)x->data)->name, ent);
		if (cmp == 0)
		{
			free(ent);
			return;
		}
		else if (cmp < 0)
			x = x->left;
		else
			x = x->right;
	}

	node_t *z = malloc(sizeof(node_t));
	z->data = malloc(sizeof(ent_t));
	((ent_t *)z->data)->counts = NULL;
	((ent_t *)z->data)->sources = NIL;
	((ent_t *)z->data)->name = ent;
	z->parent = y;
	if (y == NIL)
		(*root) = z;
	else if (strcmp(((ent_t *)y->data)->name, ent) < 0)
		y->left = z;
	else
		y->right = z;
	z->left = NIL;
	z->right = NIL;
	z->color = RED;
	tree_insert_fixup(root, z);
}

void entities_delete(node_t **root, node_t *z)
{
	sources_dispose(((ent_t *)z->data)->sources);

	count_t *c;
	while (((ent_t *)z->data)->counts != NULL)
	{
		c = ((ent_t *)z->data)->counts;
		((ent_t *)z->data)->counts = c->next;
		free(c);
	}

	free(((ent_t *)z->data)->name);
	free((ent_t *)z->data);

	node_t *y, *x;
	if (z->left == NIL || z->right == NIL)
		y = z;
	else
	{
		y = tree_successor(z);
	}
	if (y->left != NIL)
		x = y->left;
	else
		x = y->right;
	x->parent = y->parent;
	if (y->parent == NIL)
		(*root) = x;
	else if (y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;
	if (y != z)
		z->data = y->data;

	if (y->color == BLACK)
		tree_delete_fixup(root, x);

	free(y);
}

void entities_dispose(node_t *x)
{
	if (x != NIL)
	{
		entities_dispose(x->left);
		entities_dispose(x->right);

		sources_dispose(((ent_t *)x->data)->sources);

		count_t *c;
		while (((ent_t *)x->data)->counts != NULL)
		{
			c = ((ent_t *)x->data)->counts;
			((ent_t *)x->data)->counts = c->next;
			free(c);
		}

		free(((ent_t *)x->data)->name);
		free((ent_t *)x->data);
		free(x);
	}
}


// sources tree operations
node_t *sources_search(node_t *node, char *ent)
{
	node_t *curr = node;
	while (curr != NIL)
	{
		int cmp = strcmp(ent, (((src_t *)curr->data)->src));
		if (cmp == 0)
			return curr;
		if (cmp < 0)
			curr = curr->left;
		else
			curr = curr->right;
	}
	return NULL;
}

src_t *sources_insert(node_t **root, char *ent)
{
	node_t *y = NIL;
	node_t *x = (*root);
	while (x != NIL)
	{
		y = x;
		int cmp = strcmp(ent, ((src_t *)x->data)->src);
		if (cmp == 0)
		{
			return (src_t *)x->data;
		}
		else if (cmp < 0)
			x = x->left;
		else
			x = x->right;
	}

	node_t *z = malloc(sizeof(node_t));
	z->data = malloc(sizeof(src_t));
	((src_t *)z->data)->types = NULL;
	((src_t *)z->data)->src = ent;
	z->parent = y;
	if (y == NIL)
		(*root) = z;
	else if (strcmp(ent, ((src_t *)y->data)->src) < 0)
		y->left = z;
	else
		y->right = z;
	z->left = NIL;
	z->right = NIL;
	z->color = RED;
	tree_insert_fixup(root, z);

	return z->data;
}

void sources_delete(node_t **root, node_t *z)
{
	free((src_t *)z->data);

	node_t *y, *x;
	if (z->left == NIL || z->right == NIL)
		y = z;
	else
	{
		y = tree_successor(z);
	}
	if (y->left != NIL)
		x = y->left;
	else
		x = y->right;
	x->parent = y->parent;
	if (y->parent == NIL)
		(*root) = x;
	else if (y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;
	if (y != z)
		z->data = y->data;

	if (y->color == BLACK)
		tree_delete_fixup(root, x);

	free(y);
}

void sources_dispose(node_t *x)
{
	if (x != NIL)
	{
		sources_dispose(x->left);
		sources_dispose(x->right);

		rel_t *r;
		while (((src_t *)x->data)->types != NULL)
		{
			r = ((src_t *)x->data)->types;
			((src_t *)x->data)->types = r->next;
			free(r);
		}

		free((src_t *)x->data);
		free(x);
	}
}