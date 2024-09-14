#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

ht* ht_create(void)
{
	ht* table = malloc(sizeof(ht));
	if (table == NULL)
	{
		return NULL;
	}
	table->length = 0;
	table->capacity = INITIAL_CAPACITY;

	table->entries = calloc(table->capacity, sizeof(ht_entry));
	if (table->entries == NULL)
	{
		free(table);
		return NULL;
	}
	return table;
}

void ht_destroy(ht* table)
{
	for (int i = 0; i < table->capacity; i++)
	{
		free((void*) table->entries[i].key);
	}

	free(table->entries);
	free(table);
}

static int hash_key(const char* key)
{
	int hash = FNV_OFFSET;
	for (const char* p = key; *p; p++)
	{
		hash ^= (int) (unsigned char) (*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

void* ht_get(ht* table, const char* key)
{
	int hash = hash_key(key);
	int index = (int) (hash & (int) (table->capacity - 1));

	while (table->entries[index].key != NULL)
	{
		if (strcmp(key, table->entries[index].key) == 0)
		{
			return table->entries[index].value;
		}

		index++;
		if (index >= table->capacity)
		{
			index = 0;
		}
	}
	return NULL;
}

static const char* ht_set_entry(ht_entry* entries, int capacity,
		const char* key, void* value, int* plength)
{

	int hash = hash_key(key);
	int index = (int) (hash & (int) (capacity - 1));

	while (entries[index].key != NULL)
	{
		if (strcmp(key, entries[index].key) == 0)
		{
			entries[index].value = value;
			return entries[index].key;
		}

		index++;
		if (index >= capacity)
		{
			index = 0;
		}
	}

	if (plength != NULL)
	{
		key = _strdup(key);
		if (key == NULL)
		{
			return NULL;
		}
		(*plength)++;
	}
	entries[index].key = (char*) key;
	entries[index].value = value;
	return key;
}

const char* ht_set(ht* table, const char* key, void* value)
{
	assert(value != NULL);
	if (value == NULL)
	{
		return NULL;
	}

	
	if (table->length >= table->capacity / 2)
	{
		if (!ht_expand(table))
		{
			return NULL;
		}
	}

	return ht_set_entry(table->entries, table->capacity, key, value,
						&table->length);
}


static int ht_expand(ht* table)
{
	
	int new_capacity = table->capacity * 2;
	if (new_capacity < table->capacity)
	{
		return 0;  
	}
	ht_entry* new_entries = calloc(new_capacity, sizeof(ht_entry));
	if (new_entries == NULL)
	{
		return 0;
	}

	
	for (int i = 0; i < table->capacity; i++)
	{
		ht_entry entry = table->entries[i];
		if (entry.key != NULL)
		{
			ht_set_entry(new_entries, new_capacity, entry.key,
						 entry.value, NULL);
		}
	}

	
	free(table->entries);
	table->entries = new_entries;
	table->capacity = new_capacity;
	return 1;
}

int ht_length(ht* table)
{
	return table->length;
}

hti ht_iterator(ht* table)
{
	hti it;
	it._table = table;
	it._index = 0;
	return it;
}

int ht_next(hti* it)
{
	
	ht* table = it->_table;
	while (it->_index < table->capacity)
	{
		int i = it->_index;
		it->_index++;
		if (table->entries[i].key != NULL)
		{
			
			ht_entry entry = table->entries[i];
			it->key = entry.key;
			it->value = entry.value;
			return 1;
		}
	}
	return 0;
}