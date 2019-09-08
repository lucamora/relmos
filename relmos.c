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

typedef struct el
{
	char *src;
	type_t *type;
	struct el *next;
} src_t;

typedef struct cnt
{
	int count;
	type_t *type;
	struct cnt *next;
} count_t;

typedef struct
{
	char *dst;
	count_t *counts;
	src_t *sources;
} dst_t;

typedef struct del
{
	node_t *node;
	struct del *next;
} del_t;


void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();
void end();


// private helper function
src_t *_getprevsrc(char *src, type_t *rel, src_t *sources);
type_t *_getreltype(char *name);
void _delsrc(char *name, type_t *rel, dst_t *curr_dst);
void _finddeldst(node_t *x, char *ent, del_t **list);
void _increment(dst_t *dst, type_t *type);
void _decrement(dst_t *dst, type_t *type);
int _buildreport(node_t *x);


// tree operations
void tree_left_rotate(node_t **root, node_t *x);
void tree_right_rotate(node_t **root, node_t *x);
void tree_insert_fixup(node_t **root, node_t *z);
void tree_delete_fixup(node_t **root, node_t *x);
node_t *tree_minimum(node_t *x);
node_t *tree_successor(node_t *x);

node_t *entities_search(node_t *node, char *ent);
int entities_insert(node_t **root, char *ent);
void entities_delete(node_t **root, node_t *z);
void entities_dispose(node_t *x);

node_t *destinations_search(node_t *node, char *ent);
dst_t *destinations_insert(node_t **root, char *ent);
void destinations_delete(node_t **root, node_t *z);
void destinations_dispose(node_t *x);

#define NIL (&nil)
static node_t nil = { NULL, BLACK, &nil, &nil, &nil };


// main data structures
node_t *entities = NIL;
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
	entities_insert(&entities, entity);
}

void delent(char *entity)
{
	// lookup entity
	node_t *curr_ent = entities_search(entities, entity);

	free(entity);

	// entity does not exist
	if (curr_ent == NULL)
		return;

	// get a list of destinations to delete
	del_t *delete = NULL;
	_finddeldst(destinations, (char *)curr_ent->data, &delete);

	// delete destinations
	del_t *i;
	while (delete != NULL)
	{
		destinations_delete(&destinations, delete->node);
		i = delete;
		delete = i->next;
		free(i);
	}

	// delete entity
	entities_delete(&entities, curr_ent);
}

void addrel(char *src, char *dst, char *rel)
{
	// search if relation type already exists
	type_t *curr_type = types;
	type_t *ins_type = NULL;
	char found_type = 0;
	while (curr_type != NULL && found_type == 0)
	{
		int cmp = strcmp(rel, curr_type->name);
		if (cmp == 0)
		{
			found_type = 1;
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
	if (found_type == 0)
	{
		type_t *new_type = malloc(sizeof(type_t));
		new_type->name = rel;
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

		curr_type = new_type;
	}
	else
	{
		free(rel);
	}

	// lookup entities
	node_t *src_ent = entities_search(entities, src);
	node_t *dst_ent = entities_search(entities, dst);

	free(src);
	free(dst);

	// an entity does not exist
	if (src_ent == NULL || dst_ent == NULL)
		return;

	// create destination (if does not already exist)
	dst_t *curr_dst = destinations_insert(&destinations, (char *)dst_ent->data);

	// create source (if does not already exist)
	src_t *prev_src = _getprevsrc((char *)src_ent->data, curr_type, curr_dst->sources);
	src_t *curr_src = (prev_src != NULL) ? prev_src->next : curr_dst->sources;

	// relation already exists
	if (curr_src != NULL)
		return;

	// create source
	src_t *new_src = malloc(sizeof(src_t));
	new_src->src = (char *)src_ent->data;
	new_src->type = curr_type;
	new_src->next = curr_dst->sources;
	curr_dst->sources = new_src;
	_increment(curr_dst, curr_type);
}

void delrel(char *src, char *dst, char *rel)
{
	// lookup relation type
	type_t *relation = _getreltype(rel);
	free(rel);

	// lookup entities
	node_t *src_ent = entities_search(entities, src);
	node_t *dst_ent = entities_search(entities, dst);

	free(src);
	free(dst);

	// relation or an entity does not exist
	if (relation == NULL || src_ent == NULL || dst_ent == NULL)
		return;

	node_t *curr_dst = destinations_search(destinations, (char *)dst_ent->data);

	// destination does not exist
	if (curr_dst == NULL)
		return;
	
	// delete source
	_delsrc((char *)src_ent->data, relation, (dst_t *)curr_dst->data);

	// delete destination if does not have any source
	if (((dst_t *)curr_dst)->sources == NULL)
	{
		destinations_delete(&destinations, curr_dst);
	}
}

void report()
{
	// number of items added to the report
	int added = 0;
	if (types != NULL)
	{
		added = _buildreport(destinations);
	}

	if (added > 0)
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
	destinations_dispose(destinations);

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

	// free entities
	entities_dispose(entities);
}


// private helper function
src_t *_getprevsrc(char *src, type_t *rel, src_t *sources)
{
	src_t *curr = sources;
	src_t *prev = NULL;
	while (curr != NULL)
	{
		if (src == curr->src && rel == curr->type)
		{
			return prev;
		}
		else
		{
			prev = curr;
			curr = curr->next;
		}
	}
	return prev;
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

void _delsrc(char *name, type_t *rel, dst_t *curr_dst)
{
	src_t *curr = curr_dst->sources;
	src_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->src) == 0)
		{
			if (rel != NULL)
			{
				// remove only a relation
				if (rel == curr->type)
				{
					// remove source
					if (prev == NULL)
					{
						curr_dst->sources = curr->next;
					}
					else
					{
						prev->next = curr->next;
					}

					_decrement(curr_dst, curr->type);
					free(curr);
					return;
				}
				else
				{
					prev = curr;
					curr = curr->next;
				}
			}
			else
			{
				// remove all relations
				// remove source
				if (prev == NULL)
				{
					curr_dst->sources = curr->next;
				}
				else
				{
					prev->next = curr->next;
				}

				_decrement(curr_dst, curr->type);
				free(curr);
				curr = (prev != NULL) ? prev->next : curr_dst->sources;
			}
		}
		else
		{
			prev = curr;
			curr = curr->next;
		}
	}
}

void _finddeldst(node_t *x, char *ent, del_t **list)
{
	if (x == NIL)
		return;

	if (((dst_t *)x->data)->dst == ent)
	{
		del_t *del = malloc(sizeof(del_t));
		del->node = x;
		del->next = *list;
		*list = del;
	}
	else
	{
		_delsrc(ent, NULL, (dst_t *)x->data);

		if (((dst_t *)x->data)->sources == NULL)
		{
			del_t *del = malloc(sizeof(del_t));
			del->node = x;
			del->next = *list;
			*list = del;
		}
	}

	_finddeldst(x->left, ent, list);
	_finddeldst(x->right, ent, list);
}

void _increment(dst_t *dst, type_t *type)
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

void _decrement(dst_t *dst, type_t *type)
{
	count_t *curr = dst->counts;
	while (curr != NULL)
	{
		if (type == curr->type)
		{
			curr->count--;
			return;
		}
		curr = curr->next;
	}
}

int _buildreport(node_t *x)
{
	if (x == NIL)
		return 0;
	
	int added = _buildreport(x->right);

	count_t *count = ((dst_t *)x->data)->counts;
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
					iter->dsts->name = ((dst_t *)x->data)->dst;
					added += 1;
				}
				// if current count is equal
				// add the current dst
				else if (count->count > 0 && count->count == iter->count)
				{
					max_t *new = malloc(sizeof(max_t));
					new->name = ((dst_t *)x->data)->dst;
					new->next = iter->dsts;
					iter->dsts = new;
				}

				done = 1;
			}

			iter = iter->next;
		}

		count = count->next;
	}

	return added + _buildreport(x->left);
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
	if (z == *root)
		(*root)->color = BLACK;
	else
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
					tree_insert_fixup(root, x->parent);
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
					tree_insert_fixup(root, x->parent);
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
}

void tree_delete_fixup(node_t **root, node_t *x)
{
	if (x->color == RED || x->parent == NIL)
		x->color = BLACK;
	else if (x == x->parent->left)
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
			tree_delete_fixup(root, x->parent);
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
			tree_delete_fixup(root, x->parent);
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
		}
	}
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


// entities tree operations
node_t *entities_search(node_t *node, char *ent)
{
	node_t *curr = node;
	while (curr != NIL)
	{
		int cmp = strcmp(ent, ((char *)curr->data));
		if (cmp == 0)
			return curr;
		if (cmp < 0)
			curr = curr->left;
		else
			curr = curr->right;
	}
	return NULL;
}

int entities_insert(node_t **root, char *ent)
{
	node_t *y = NIL;
	node_t *x = (*root);
	while (x != NIL)
	{
		y = x;
		int cmp = strcmp(ent, (char *)x->data);
		if (cmp == 0)
		{
			free(ent);
			return 0;
		}
		else if (cmp < 0)
			x = x->left;
		else
			x = x->right;
	}

	node_t *z = malloc(sizeof(node_t));
	z->data = ent;
	z->parent = y;
	if (y == NIL)
		(*root) = z;
	else if (strcmp(ent, (char *)y->data) < 0)
		y->left = z;
	else
		y->right = z;
	z->left = NIL;
	z->right = NIL;
	z->color = RED;
	tree_insert_fixup(root, z);

	return 1;
}

void entities_delete(node_t **root, node_t *z)
{
	free((char *)z->data);

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
		free(x->data);
		free(x);
	}
}


// destinations tree operations
node_t *destinations_search(node_t *node, char *ent)
{
	node_t *curr = node;
	while (curr != NIL)
	{
		int cmp = strcmp(ent, (((dst_t *)curr->data)->dst));
		if (cmp == 0)
			return curr;
		if (cmp < 0)
			curr = curr->left;
		else
			curr = curr->right;
	}
	return NULL;
}

dst_t *destinations_insert(node_t **root, char *ent)
{
	node_t *y = NIL;
	node_t *x = (*root);
	while (x != NIL)
	{
		y = x;
		int cmp = strcmp(ent, ((dst_t *)x->data)->dst);
		if (cmp == 0)
		{
			return (dst_t *)x->data;
		}
		else if (cmp < 0)
			x = x->left;
		else
			x = x->right;
	}

	node_t *z = malloc(sizeof(node_t));
	z->data = malloc(sizeof(dst_t));
	((dst_t *)z->data)->counts = NULL;
	((dst_t *)z->data)->sources = NULL;
	((dst_t *)z->data)->dst = ent;
	z->parent = y;
	if (y == NIL)
		(*root) = z;
	else if (strcmp(ent, ((dst_t *)y->data)->dst) < 0)
		y->left = z;
	else
		y->right = z;
	z->left = NIL;
	z->right = NIL;
	z->color = RED;
	tree_insert_fixup(root, z);

	return z->data;
}

void destinations_delete(node_t **root, node_t *z)
{
	src_t *s;
	while (((dst_t *)z->data)->sources != NULL)
	{
		s = ((dst_t *)z->data)->sources;
		((dst_t *)z->data)->sources = s->next;
		free(s);
	}

	count_t *c;
	while (((dst_t *)z->data)->counts != NULL)
	{
		c = ((dst_t *)z->data)->counts;
		((dst_t *)z->data)->counts = c->next;
		free(c);
	}

	free((dst_t *)z->data);

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

void destinations_dispose(node_t *x)
{
	if (x != NIL)
	{
		destinations_dispose(x->left);
		destinations_dispose(x->right);

		src_t *s;
		while (((dst_t *)x->data)->sources != NULL)
		{
			s = ((dst_t *)x->data)->sources;
			((dst_t *)x->data)->sources = s->next;
			free(s);
		}

		count_t *c;
		while (((dst_t *)x->data)->counts != NULL)
		{
			c = ((dst_t *)x->data)->counts;
			((dst_t *)x->data)->counts = c->next;
			free(c);
		}

		free((dst_t *)x->data);
		free(x);
	}
}