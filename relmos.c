#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

typedef struct ent
{
	char *name;
	struct ent *next;
} ent_t;

typedef struct typ
{
	char *name;
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
	//int count; // remove
	count_t *counts;
	src_t *sources;
	struct elem *next;
} dst_t;

/*typedef struct rel // remove
{
	char *name;
	dst_t *instances;
	struct rel *next;
} rel_type_t;*/

typedef struct maxd
{
	char *name;
	struct maxd *next;
} max_dst_t;

typedef struct maxt
{
	char *name;
	int count;
	max_dst_t *dsts;
	struct maxt *next;
} max_type_t;

ent_t *entities = NULL;
//rel_type_t *relations = NULL;
dst_t *destinations = NULL;
type_t *types = NULL;

void addent(char *entity);
void delent(char *entity);
void addrel(char *src, char *dst, char *rel);
void delrel(char *src, char *dst, char *rel);
void report();

ent_t *_getent(char *name);
ent_t *_getprevent(char *name);
dst_t *_getprevdst(char *name, dst_t *relation);
src_t *_getprevsrc(char *name, type_t *rel, src_t *sources);
type_t *_getreltype(char *name);
void _delsrc(char *name, type_t *rel, dst_t *curr_dst);
void _deldst(dst_t *prev, dst_t *curr, dst_t **relation);
void increment(dst_t *dst, type_t *type);
void decrement(dst_t *dst, type_t *type);

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
	//rel_type_t *rel_type = relations;
	//while (rel_type != NULL)
	//{
		//dst_t *curr_dst = rel_type->instances;
		dst_t *curr_dst = destinations;
		dst_t *prev_dst = NULL;
		while (curr_dst != NULL)
		{
			if (strcmp(entity, curr_dst->dst->name) == 0)
			{
				// remove from destinations
				//_deldst(prev_dst, curr_dst, &rel_type->instances);
				_deldst(prev_dst, curr_dst, &destinations);

				// if there is a previous dst
				// current is its next
				// otherwise current is the first element
				//curr_dst = (prev_dst != NULL) ? prev_dst->next : rel_type->instances;
				curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;
			}
			else
			{
				// remove from sources
				_delsrc(entity, NULL, curr_dst);
				
				//if (curr_dst->count == 0)
				if (curr_dst->sources == NULL)
				{
					// all relations to this entity have been deleted
					// remove from destinations
					//_deldst(prev_dst, curr_dst, &rel_type->instances);
					_deldst(prev_dst, curr_dst, &destinations);

					// if there is a previous dst
					// current is its next
					// otherwise current is the first element
					//curr_dst = (prev_dst != NULL) ? prev_dst->next : rel_type->instances;
					curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;
				}
				else
				{
					prev_dst = curr_dst;
					curr_dst = curr_dst->next;
				}
			}
		}

		//rel_type = rel_type->next;
	//}

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
			if (cmp < 0)
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

	/*rel_type_t *curr_type = relations;
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
	}*/

	ent_t *src_ent = _getent(src);
	ent_t *dst_ent = _getent(dst);

	if (src_ent == NULL || dst_ent == NULL) {
		return;
	}

	free(src);
	free(dst);

	//dst_t *curr_dst = curr_type->instances;
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
		//new_dst->count = 0;
		new_dst->counts = NULL;
		new_dst->sources = NULL;

		if (ins == NULL)
		{
			//new_dst->next = curr_type->instances;
			//curr_type->instances = new_dst;
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

	if (curr_src != NULL) {
		// relation already exists
		return;
	}
	
	src_t *new_src = malloc(sizeof(src_t));
	new_src->src = src_ent;
	new_src->type = curr_type;
	new_src->next = curr_dst->sources;
	curr_dst->sources = new_src;
	increment(curr_dst, curr_type);
	//curr_dst->count = curr_dst->count + 1;
}

void delrel(char *src, char *dst, char *rel)
{
	type_t *relation = _getreltype(rel);
	free(rel);

	if (relation == NULL) {
		// relation does not exist
		return;
	}

	//dst_t *prev_dst = _getprevdst(dst, relation->instances);
	//dst_t *curr_dst = (prev_dst != NULL) ? prev_dst->next : relation->instances;
	dst_t *prev_dst = _getprevdst(dst, destinations);
	dst_t *curr_dst = (prev_dst != NULL) ? prev_dst->next : destinations;

	free(dst);

	if (curr_dst != NULL)
	{
		_delsrc(src, relation, curr_dst);

		free(src);

		//if (curr_dst->count == 0)
		if (curr_dst->sources == NULL)
		{
			//_deldst(prev_dst, curr_dst, &relation->instances);
			_deldst(prev_dst, curr_dst, &destinations);
		}
	}
}

void report()
{
	char empty = 1;
	char first = 1;

	/*rel_type_t *curr_type = relations;
	while (curr_type != NULL)
	{
		if (curr_type->instances != NULL)
		{
			empty = 0;
			dst_t *relation = curr_type->instances;

			max_t *dsts = malloc(sizeof(max_t));
			dsts->name = relation->dst->name;
			dsts->next = NULL;
			int max = relation->count;

			dst_t *curr = relation->next;
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
	}*/

	// create ordered list of relation types
	max_type_t *relations = NULL;
	type_t *curr_type = types;
	while (curr_type != NULL)
	{
		max_type_t *tmp = malloc(sizeof(max_type_t));
		tmp->count = 0;
		tmp->dsts = NULL;
		tmp->name = curr_type->name;
		tmp->next = relations;
		relations = tmp;
		
		/*max_type_t *iter = relations;
		max_type_t *ins = NULL;
		while (iter != NULL)
		{
			int cmp = strcmp(tmp->name, iter->name);
			if (cmp > 0)
			{
				ins = iter;
			}
			iter = iter->next;
		}

		if (ins == NULL)
		{
			tmp->next = relations;
			relations = tmp;
		}
		else
		{
			tmp->next = ins->next;
			ins->next = tmp;
		}*/

		curr_type = curr_type->next;
	}

	if (relations != NULL)
	{
		// scan through destinations to fill types list
		dst_t *curr_dst = destinations;
		while (curr_dst != NULL)
		{
			count_t *curr_count = curr_dst->counts;
			while (curr_count != NULL)
			{
				max_type_t *iter = relations;
				char done = 0;
				while (done == 0)
				{
					if (strcmp(iter->name, curr_count->type->name) == 0)
					{
						if (curr_count->count > iter->count)
						{
							iter->count = curr_count->count;

							max_dst_t *c;
							while (iter->dsts != NULL)
							{
								c = iter->dsts;
								iter->dsts = c->next;
								free(c);
							}
							
							iter->dsts = malloc(sizeof(max_dst_t));
							iter->dsts->next = NULL;
							iter->dsts->name = curr_dst->dst->name;
							empty = 0;
						}
						else if (curr_count->count > 0 && curr_count->count == iter->count)
						{
							max_dst_t *new = malloc(sizeof(max_dst_t));
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
		max_type_t *tmp = relations;
		while (relations != NULL)
		{
			if (relations->count > 0)
			{
				if (first == 0) {
					printf(" ");
				}
				first = 0;
				printf("%s ", relations->name);

				max_dst_t *i = relations->dsts;
				while (relations->dsts != NULL)
				{
					printf("%s ", relations->dsts->name);

					i = relations->dsts;
					relations->dsts = i->next;
					free(i);
				}

				printf("%d;", relations->count);
			}
			
			tmp = relations;
			relations = tmp->next;
			free(tmp);
		}
		/*max_t *tmp;
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

		printf("%d;", max);*/
	}
	else
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
	/*rel_type_t *curr = relations;
	while (curr != NULL)
	{
		if (strcmp(name, curr->name) == 0)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;*/
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
	/*
	src_t *prev_src = _getprevsrc(name, rel, curr_dst->sources);
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

		//curr_dst->count = curr_dst->count - 1;
		decrement(curr_dst, curr_src->type);

		free(curr_src);
	}*/

	src_t *curr = curr_dst->sources;
	src_t *prev = NULL;
	while (curr != NULL)
	{
		if (strcmp(name, curr->src->name) == 0)
		{
			if (rel != NULL) {
				// remove only a relation
				if (strcmp(rel->name, curr->type->name) == 0) {
					// remove source
					if (prev == NULL)
					{
						curr_dst->sources = curr->next;
					}
					else
					{
						prev->next = curr->next;
					}

					decrement(curr_dst, curr->type);
					free(curr);
					return;
				}
				else
				{
					prev = curr;
					curr = curr->next;
				}
			}
			else {
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

				decrement(curr_dst, curr->type);
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

	src_t *c;
	while (curr->sources != NULL)
	{
		c = curr->sources;
		curr->sources = c->next;
		free(c);
	}

	free(curr);
}

void increment(dst_t *dst, type_t *type)
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

void decrement(dst_t *dst, type_t *type)
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