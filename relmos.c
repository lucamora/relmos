#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

typedef struct ent
{
	char *name;
	struct ent *next;
} ent_t;

typedef struct node
{
	ent_t *src;
	struct node *next;
} src_t;

typedef struct elem
{
	ent_t *dst;
	int count;
	src_t *sources;
	struct elem *next;
} rel_t;

typedef struct rel
{
	char *name;
	rel_t *instances;
	struct rel *next;
} rel_type_t;

typedef struct max
{
	char *name;
	struct max *next;
} max_t;

ent_t *entities = NULL;
rel_type_t *relations = NULL;

void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();

ent_t *_getent(char *name);
ent_t *_getprevent(char *name);
rel_t *_getprevdst(char *name, rel_t *relation);
src_t *_getprevsrc(char *name, src_t *sources);
rel_type_t *_getreltype(char *name);
void _delsrc(char *name, rel_t *curr_dst);
void _deldst(rel_t *prev, rel_t *curr, rel_t **relation);

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
			return 0;
		}
	} while (1);
}

void addent(char *entity)
{
	ent_t *found = _getent(entity);

	if (found != NULL) {
		// entity already exists
		return;
	}

	// create and insert new entity
	ent_t *tmp = malloc(sizeof(ent_t));
	tmp->name = entity;
	tmp->next = entities;
	entities = tmp;
}

void delent(char *entity)
{
	ent_t *prev_ent = _getprevent(entity);
	ent_t *curr_ent = (prev_ent != NULL) ? prev_ent->next : entities;

	if (curr_ent == NULL) {
		// entity does not exist
		return;
	}

	// remove entity from every relation
	rel_type_t *rel_type = relations;
	while (rel_type != NULL)
	{
		rel_t *curr_dst = rel_type->instances;
		rel_t *prev_dst = NULL;
		while (curr_dst != NULL)
		{
			if (strcmp(entity, curr_dst->dst->name) == 0)
			{
				// remove from destinations
				_deldst(prev_dst, curr_dst, &rel_type->instances);

				// if there is a previous dst
				// current is its next
				// otherwise current is the first element
				curr_dst = (prev_dst != NULL) ? prev_dst->next : rel_type->instances;
			}
			else
			{
				// remove from sources
				_delsrc(entity, curr_dst);
				
				if (curr_dst->count == 0)
				{
					// all relations to this entity have been deleted
					// remove from destinations
					_deldst(prev_dst, curr_dst, &rel_type->instances);

					// if there is a previous dst
					// current is its next
					// otherwise current is the first element
					curr_dst = (prev_dst != NULL) ? prev_dst->next : rel_type->instances;
				}
				else
				{
					prev_dst = curr_dst;
					curr_dst = curr_dst->next;
				}
			}
		}

		rel_type = rel_type->next;
	}

	// remove entity
	if (prev_ent == NULL)
	{
		entities = curr_ent->next;
	}
	else
	{
		prev_ent->next = curr_ent->next;
	}

	free(curr_ent->name);
	free(curr_ent);
	free(entity);
}

void addrel(char *src, char *dst, char *rel)
{
	// search if relation type already exists
	rel_type_t *curr_type = relations;
	rel_type_t *ins_type = NULL;
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
		rel_type_t *new_type = malloc(sizeof(rel_type_t));
		new_type->name = rel;
		new_type->instances = NULL;
		if (ins_type == NULL)
		{
			new_type->next = relations;
			relations = new_type;
		}
		else
		{
			new_type->next = ins_type->next;
			ins_type->next = new_type;
		}
		
		curr_type = new_type;
	}

	ent_t *src_ent = _getent(src);
	ent_t *dst_ent = _getent(dst);

	if (src_ent == NULL || dst_ent == NULL) {
		return;
	}

	free(src);
	free(dst);

	rel_t *curr_dst = curr_type->instances;
	rel_t *ins = NULL;
	char found_dst = 0;
	while (curr_dst != NULL && found_dst == 0)
	{
		int cmp = strcmp(dst_ent->name, curr_dst->dst->name);
		if (cmp == 0)
		{
			found_dst = 1;
		}
		else
		{
			if (cmp < 0)
			{
				ins = curr_dst;
			}
			curr_dst = curr_dst->next;	
		}
	}

	if (found_dst == 0)
	{
		rel_t *new_dst = malloc(sizeof(rel_t));
		new_dst->dst = dst_ent;
		new_dst->count = 0;
		new_dst->sources = NULL;

		if (ins == NULL)
		{
			new_dst->next = curr_type->instances;
			curr_type->instances = new_dst;
		}
		else
		{
			new_dst->next = ins->next;
			ins->next = new_dst;
		}

		curr_dst = new_dst;
	}

	src_t *prev_src = _getprevsrc(src_ent->name, curr_dst->sources);
	src_t *curr_src = (prev_src != NULL) ? prev_src->next : curr_dst->sources;

	if (curr_src != NULL) {
		// relation already exists
		return;
	}
	
	src_t *new_src = malloc(sizeof(src_t));
	new_src->src = src_ent;
	new_src->next = curr_dst->sources;
	curr_dst->sources = new_src;
	curr_dst->count = curr_dst->count + 1;
}

void delrel(char *src, char *dst, char *rel)
{
	rel_type_t *relation = _getreltype(rel);
	free(rel);

	if (relation == NULL) {
		// relation does not exist
		return;
	}

	rel_t *prev_dst = _getprevdst(dst, relation->instances);
	rel_t *curr_dst = (prev_dst != NULL) ? prev_dst->next : relation->instances;

	free(dst);

	if (curr_dst != NULL)
	{
		_delsrc(src, curr_dst);

		free(src);

		if (curr_dst->count == 0)
		{
			_deldst(prev_dst, curr_dst, &relation->instances);
		}
	}
}

void report()
{
	char empty = 1;
	char first = 1;

	rel_type_t *curr_type = relations;
	while (curr_type != NULL)
	{
		if (curr_type->instances != NULL)
		{
			empty = 0;
			rel_t *relation = curr_type->instances;

			max_t *dsts = malloc(sizeof(max_t));
			dsts->name = relation->dst->name;
			dsts->next = NULL;
			int max = relation->count;

			rel_t *curr = relation->next;
			while (curr != NULL)
			{
				if (curr->count > max)
				{
					max = curr->count;

					max_t *c;
					while (dsts->next != NULL)
					{
						c = dsts;
						dsts = c->next;
						free(c);
					}
					
					dsts->name = curr->dst->name;
				}
				else if (curr->count == max)
				{
					max_t *new = malloc(sizeof(max_t));
					new->name = curr->dst->name;
					new->next = dsts;
					dsts = new;
				}

				curr = curr->next;
			}

			max_t *tmp;
			if (first == 0) {
				printf(" ");
			}
			first = 0;

			printf("%s ", curr_type->name);
			while (dsts != NULL)
			{
				printf("%s ", dsts->name);

				tmp = dsts;
				dsts = tmp->next;
				free(tmp);
			}

			printf("%d;", max);
		}

		curr_type = curr_type->next;
	}

	if (empty == 1)
	{
		printf("none");
	}

	printf("\n");
}

ent_t *_getent(char *name)
{
	ent_t *curr = entities;
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

ent_t *_getprevent(char *name)
{
	ent_t *curr = entities;
	ent_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->name) == 0)
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

rel_t *_getprevdst(char *name, rel_t *relation)
{
	rel_t *curr = relation;
	rel_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->dst->name) == 0)
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

src_t *_getprevsrc(char *name, src_t *sources)
{
	src_t *curr = sources;
	src_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->src->name) == 0)
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

rel_type_t *_getreltype(char *name)
{
	rel_type_t *curr = relations;
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

void _delsrc(char *name, rel_t *curr_dst)
{
	src_t *prev_src = _getprevsrc(name, curr_dst->sources);
	src_t *curr_src = (prev_src != NULL) ? prev_src->next : curr_dst->sources;

	if (curr_src != NULL)
	{
		if (prev_src == NULL)
		{
			curr_dst->sources = curr_src->next;
		}
		else
		{
			prev_src->next = curr_src->next;
		}

		free(curr_src);

		curr_dst->count = curr_dst->count - 1;
	}
}

void _deldst(rel_t *prev, rel_t *curr, rel_t **relation)
{
	if (prev == NULL)
	{
		*relation = curr->next;
	}
	else
	{
		prev->next = curr->next;
	}

	src_t *c;
	while (curr->sources != NULL)
	{
		c = curr->sources;
		curr->sources = c->next;
		free(c);
	}

	free(curr);
}
