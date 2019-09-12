#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 40
#define RED 'r'
#define BLACK 'b'

#define TYPE_DSTS_SIZE 600
#define TYPES_SIZE 5

typedef struct node
{
	void *data;
	char color;
	struct node *left, *right, *parent;
} node_t;

typedef struct typ
{
	char *name;
	int count;
	char *dsts[TYPE_DSTS_SIZE];
	int dstscount;
} type_t;

typedef struct el
{
	char *src;
	char types[TYPES_SIZE];
	int typescount;
} src_t;

typedef struct
{
	char *name;
	int counts[TYPES_SIZE];
	node_t *sources;
} ent_t;


void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();
void end();


// private helper function
int _addreltype(char *name);
int _getreltype(char *name);
void _delsrc(char *name, int rel, ent_t *curr_dst);
void _delsources(node_t *x, char *ent);
void _increment(ent_t *dst, int type);
void _decrement(ent_t *dst, int type);
int _buildreport(node_t *x);


// tree operations
void tree_left_rotate(node_t **root, node_t *x);
void tree_right_rotate(node_t **root, node_t *x);
void tree_insert_fixup(node_t **root, node_t *z);
void tree_delete_fixup(node_t **root, node_t *x);
node_t *tree_minimum(node_t *x);
node_t *tree_successor(node_t *x);

#define NIL (&nil)
static node_t nil = { NULL, BLACK, &nil, &nil, &nil };

node_t *entities_search(node_t *root, char *ent);
void entities_insert(node_t **root, char *ent);
void entities_delete(node_t **root, node_t *z);
void entities_dispose(node_t *root);

node_t *sources_search(node_t *node, char *ent);
src_t *sources_insert(node_t **root, char *ent);
void sources_delete(node_t **root, node_t *z);
void sources_dispose(node_t *x);


// main data structures
node_t *entities = NIL;
struct {
	type_t *list[TYPES_SIZE];
	int count;
} types;

struct {
	type_t *list[TYPES_SIZE];
	int lastcount;
} ordered;

int main(int argc, char const *argv[])
{
	types.count = 0;
	ordered.lastcount = 0;

	char cmd[7];
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
				src = malloc(MAX_INPUT);
				scanf("%s", src);

				addent(src);
			}
			else
			{
				// addrel
				src = malloc(MAX_INPUT);
				dst = malloc(MAX_INPUT);
				rel = malloc(MAX_INPUT);
				scanf("%s%*c%s%*c%s", src, dst, rel);

				addrel(src, dst, rel);
			}
		}
		else if (cmd[0] == 'd')
		{
			// delent or delrel
			if (cmd[3] == 'e')
			{
				// delent
				src = malloc(MAX_INPUT);
				scanf("%s", src);

				delent(src);
			}
			else
			{
				// delrel
				src = malloc(MAX_INPUT);
				dst = malloc(MAX_INPUT);
				rel = malloc(MAX_INPUT);
				scanf("%s%*c%s%*c%s", src, dst, rel);

				delrel(src, dst, rel);
			}
		}
		else if (cmd[0] == 'r')
		{
			// report
			report();
		}
		else if (cmd[0] == 'e')
		{
			// end
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

	// delete all occurrencies in other sources
	_delsources(entities, ((ent_t *)curr_ent->data)->name);

	// delete entity
	entities_delete(&entities, curr_ent);
}

void addrel(char *src, char *dst, char *rel)
{
	// lookup entities
	node_t *src_ent = entities_search(entities, src);
	free(src);
	if (src_ent == NULL)
	{
		free(dst);
		free(rel);
		return;
	}
	
	node_t *dst_ent = entities_search(entities, dst);
	free(dst);
	if (dst_ent == NULL)
	{
		free(rel);
		return;
	}

	// search if relation type already exists
	int curr_type = _addreltype(rel);

	// create source (if does not already exist)
	src_t *curr_src = sources_insert(&((ent_t *)dst_ent->data)->sources, ((ent_t *)src_ent->data)->name);

	// lookup relation
	if (curr_src->types[curr_type] > 0)
		return;

	// create relation	
	curr_src->types[curr_type] = 1;
	curr_src->typescount = curr_src->typescount + 1;
	_increment((ent_t *)dst_ent->data, curr_type);
}

void delrel(char *src, char *dst, char *rel)
{
	// lookup relation type
	int relation = _getreltype(rel);
	free(rel);
	if (relation < 0)
	{
		free(src);
		free(dst);
		return;
	}

	// lookup entities
	node_t *src_ent = entities_search(entities, src);
	free(src);
	if (src_ent == NULL)
	{
		free(dst);
		return;
	}

	node_t *dst_ent = entities_search(entities, dst);
	free(dst);
	if (dst_ent == NULL)
		return;
	
	// delete source
	_delsrc(((ent_t *)src_ent->data)->name, relation, (ent_t *)dst_ent->data);
}

void report()
{
	// number of items added to the report
	int added = 0;
	if (types.count > 0)
	{
		added = _buildreport(entities);
	}

	if (added > 0)
	{
		char first = 1;

		// compare with last ordered and reorder only if necessary
		if (types.count != ordered.lastcount)
		{
			ordered.lastcount = types.count;

			for (int i = 0; i < types.count; i++)
				ordered.list[i] = types.list[i];

			// sort types (insertion sort)
			for (int j = 1; j < types.count; j++)
			{
				type_t *t = ordered.list[j];
				int i = j - 1;
				while (i >= 0 && (strcmp(ordered.list[i]->name, t->name) > 0))
				{
					ordered.list[i + 1] = ordered.list[i];
					i = i - 1;
				}
				ordered.list[i + 1] = t;
			}
		}
		
		for (int t = 0; t < types.count; t++)
		{
			if (ordered.list[t]->count > 0)
			{
				type_t *type = ordered.list[t];
				if (first == 0)
					printf(" ");
				first = 0;
				printf("%s ", type->name);

				for (int m = type->dstscount - 1; m >= 0; m--)
				{
					printf("%s ", type->dsts[m]);
				}

				printf("%d;", type->count);

				type->dstscount = 0;
				type->count = 0;
			}

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
	// free entities
	entities_dispose(entities);

	// free types
	for (int t = 0; t < types.count; t++)
	{
		free(types.list[t]->name);
		free(types.list[t]);
	}
}


// private helper function
int _addreltype(char *name)
{
	for (int i = 0; i < types.count; i++)
	{
		int cmp = strcmp(name, types.list[i]->name);
		if (cmp == 0)
		{
			free(name);
			return i;
		}
	}

	type_t *new = malloc(sizeof(type_t));
	new->name = name;
	new->count = 0;
	new->dstscount = 0;
	
	types.list[types.count] = new;
	types.count++;

	return types.count - 1;
}

int _getreltype(char *name)
{
	for (int i = 0; i < types.count; i++)
	{
		if (strcmp(name, types.list[i]->name) == 0)
			return i;
	}
	return -1;
}

void _delsrc(char *name, int rel, ent_t *curr_dst)
{
	// if rel > -1
	// then delete only specific relation
	// else delete all relations
	node_t *curr_src = sources_search(curr_dst->sources, name);

	if (curr_src == NULL)
		return;

	if (rel > -1)
	{
		if (((src_t *)curr_src->data)->types[rel] == 1)
		{
			((src_t *)curr_src->data)->types[rel] = 0;
			((src_t *)curr_src->data)->typescount = ((src_t *)curr_src->data)->typescount - 1;
			_decrement(curr_dst, rel);
		}

		// delete source if it does not have any relation
		if (((src_t *)curr_src->data)->typescount == 0)
		{
			sources_delete(&curr_dst->sources, curr_src);
			return;
		}
	}
	else
	{
		// delete all relations
		for (int i = 0; i < types.count; i++)
		{
			if (((src_t *)curr_src->data)->types[i] == 1)
				_decrement(curr_dst, i);
		}

		// delete source
		sources_delete(&curr_dst->sources, curr_src);
	}
}

void _delsources(node_t *x, char *ent)
{
	if (x == NIL)
		return;

	if (((ent_t *)x->data)->name != ent)
		_delsrc(ent, -1, (ent_t *)x->data);

	_delsources(x->left, ent);
	_delsources(x->right, ent);
}

void _increment(ent_t *dst, int type)
{
	dst->counts[type] = dst->counts[type] + 1;
}

void _decrement(ent_t *dst, int type)
{
	dst->counts[type] = dst->counts[type] - 1;
}

int _buildreport(node_t *x)
{
	if (x == NIL)
		return 0;

	int added = _buildreport(x->left);

	// start build report
	for (int c = 0; c < types.count; c++)
	{
		int count = ((ent_t *)x->data)->counts[c];
		// if current count is bigger
		if (count > types.list[c]->count)
		{
			types.list[c]->count = count;

			types.list[c]->dsts[0] = ((ent_t *)x->data)->name;
			types.list[c]->dstscount = 1;

			added = 1;
		}
		// if current count is equal
		// add the current dst
		else if (count > 0 && count == types.list[c]->count)
		{
			types.list[c]->dsts[types.list[c]->dstscount] = ((ent_t *)x->data)->name;
			types.list[c]->dstscount = types.list[c]->dstscount + 1;
		}
	}
	// end build report

	return added + _buildreport(x->right);
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
node_t *entities_search(node_t *root, char *ent)
{
	node_t *curr = root;
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
	for (int i = 0; i < TYPES_SIZE; i++)
		((ent_t *)z->data)->counts[i] = 0;
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
	if (x == NIL)
		return;

	entities_dispose(x->left);
	entities_dispose(x->right);

	sources_dispose(((ent_t *)x->data)->sources);

	free(((ent_t *)x->data)->name);
	free((ent_t *)x->data);
	free(x);
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
	((src_t *)z->data)->src = ent;
	((src_t *)z->data)->typescount = 0;
	for (int i = 0; i < TYPES_SIZE; i++)
		((src_t *)z->data)->types[i] = 0;
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

		free((src_t *)x->data);
		free(x);
	}
}