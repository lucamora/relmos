#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

typedef struct ent
{
	char *name;
	struct ent *next;
} ent_t;

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

typedef struct node
{
	ent_t *src;
	type_t *type;
	struct node *next;
} src_t;

typedef struct cnt
{
	int count;
	type_t *type;
	struct cnt *next;
} count_t;

typedef struct elem
{
	ent_t *dst;
	count_t *counts;
	src_t *sources;
	struct elem *next;
} dst_t;

ent_t *entities = NULL;
dst_t *destinations = NULL;
type_t *types = NULL;

void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();
void end();

ent_t *_getent(char *name);
ent_t *_getprevent(char *name);
dst_t *_getprevdst(char *name, dst_t *relation);
src_t *_getprevsrc(char *name, type_t *rel, src_t *sources);
type_t *_getreltype(char *name);
void _delsrc(char *name, type_t *rel, dst_t *curr_dst);
void _deldst(dst_t *prev, dst_t *curr, dst_t **relation);
void _increment(dst_t *dst, type_t *type);
void _decrement(dst_t *dst, type_t *type);

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
	ent_t *found = _getent(entity);

	if (found != NULL)
	{
		// entity already exists
		free(entity);
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

	if (curr_ent == NULL)
	{
		// entity does not exist
		free(entity);
		return;
	}

	dst_t *curr_dst = destinations;
	dst_t *prev_dst = NULL;
	while (curr_dst != NULL)
	{
		if (strcmp(entity, curr_dst->dst->name) == 0)
		{
			// remove from destinations
			_deldst(prev_dst, curr_dst, &destinations);

			// if there is a previous dst
			// current is its next
			// otherwise current is the first element
			curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;
		}
		else
		{
			// remove from sources
			_delsrc(entity, NULL, curr_dst);

			if (curr_dst->sources == NULL)
			{
				// all relations to this entity have been deleted
				// remove from destinations
				_deldst(prev_dst, curr_dst, &destinations);

				// if there is a previous dst
				// current is its next
				// otherwise current is the first element
				curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;
			}
			else
			{
				prev_dst = curr_dst;
				curr_dst = curr_dst->next;
			}
		}
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

	ent_t *src_ent = _getent(src);
	ent_t *dst_ent = _getent(dst);

	free(src);
	free(dst);

	if (src_ent == NULL || dst_ent == NULL)
	{
		return;
	}

	dst_t *curr_dst = destinations;
	dst_t *ins = NULL;
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
		dst_t *new_dst = malloc(sizeof(dst_t));
		new_dst->dst = dst_ent;
		new_dst->counts = NULL;
		new_dst->sources = NULL;

		if (ins == NULL)
		{
			new_dst->next = destinations;
			destinations = new_dst;
		}
		else
		{
			new_dst->next = ins->next;
			ins->next = new_dst;
		}

		curr_dst = new_dst;
	}

	src_t *prev_src = _getprevsrc(src_ent->name, curr_type, curr_dst->sources);
	src_t *curr_src = (prev_src != NULL) ? prev_src->next : curr_dst->sources;

	if (curr_src != NULL)
	{
		// relation already exists
		return;
	}

	src_t *new_src = malloc(sizeof(src_t));
	new_src->src = src_ent;
	new_src->type = curr_type;
	new_src->next = curr_dst->sources;
	curr_dst->sources = new_src;
	_increment(curr_dst, curr_type);
}

void delrel(char *src, char *dst, char *rel)
{
	type_t *relation = _getreltype(rel);
	free(rel);

	if (relation == NULL)
	{
		// relation does not exist
		free(src);
		free(dst);
		return;
	}

	dst_t *prev_dst = _getprevdst(dst, destinations);
	dst_t *curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;

	if (curr_dst != NULL)
	{
		_delsrc(src, relation, curr_dst);

		if (curr_dst->sources == NULL)
		{
			_deldst(prev_dst, curr_dst, &destinations);
		}
	}

	free(src);
	free(dst);
}

void report()
{
	char empty = 1;
	if (types != NULL)
	{
		// scan through destinations to fill types list
		dst_t *curr_dst = destinations;
		while (curr_dst != NULL)
		{
			count_t *curr_count = curr_dst->counts;
			while (curr_count != NULL)
			{
				type_t *iter = types;
				char done = 0;
				while (done == 0)
				{
					if (strcmp(iter->name, curr_count->type->name) == 0)
					{
						if (curr_count->count > iter->count)
						{
							iter->count = curr_count->count;

							max_t *c;
							while (iter->dsts != NULL)
							{
								c = iter->dsts;
								iter->dsts = c->next;
								free(c);
							}

							iter->dsts = malloc(sizeof(max_t));
							iter->dsts->next = NULL;
							iter->dsts->name = curr_dst->dst->name;
							empty = 0;
						}
						else if (curr_count->count > 0 && curr_count->count == iter->count)
						{
							max_t *new = malloc(sizeof(max_t));
							new->name = curr_dst->dst->name;
							new->next = iter->dsts;
							iter->dsts = new;
						}

						done = 1;
					}

					iter = iter->next;
				}

				curr_count = curr_count->next;
			}

			curr_dst = curr_dst->next;
		}
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
				{
					printf(" ");
				}
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
	dst_t *dst;
	while (destinations != NULL)
	{
		dst = destinations;
		destinations = dst->next;
		_deldst(NULL, dst, &destinations);
	}

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
	ent_t *ent;
	while (entities != NULL)
	{
		ent = entities;
		entities = ent->next;
		free(ent->name);
		free(ent);
	}
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

		prev = curr;
		curr = curr->next;
	}
	return prev;
}

dst_t *_getprevdst(char *name, dst_t *relation)
{
	dst_t *curr = relation;
	dst_t *prev = NULL;
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

src_t *_getprevsrc(char *name, type_t *rel, src_t *sources)
{
	src_t *curr = sources;
	src_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->src->name) == 0 && strcmp(rel->name, curr->type->name) == 0)
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
		if (strcmp(name, curr->src->name) == 0)
		{
			if (rel != NULL)
			{
				// remove only a relation
				if (strcmp(rel->name, curr->type->name) == 0)
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

void _deldst(dst_t *prev, dst_t *curr, dst_t **relation)
{
	if (prev == NULL)
	{
		*relation = curr->next;
	}
	else
	{
		prev->next = curr->next;
	}

	src_t *s;
	while (curr->sources != NULL)
	{
		s = curr->sources;
		curr->sources = s->next;
		free(s);
	}

	count_t *c;
	while (curr->counts != NULL)
	{
		c = curr->counts;
		curr->counts = c->next;
		free(c);
	}

	free(curr);
}

void _increment(dst_t *dst, type_t *type)
{
	count_t *curr = dst->counts;
	while (curr != NULL)
	{
		if (strcmp(type->name, curr->type->name) == 0)
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
		if (strcmp(type->name, curr->type->name) == 0)
		{
			curr->count--;
			return;
		}
		curr = curr->next;
	}
}