#pragma once
#define INITIAL_CAPACITY 16
typedef struct {
    const char* key; 
    void* value;
} ht_entry;

typedef struct {
    ht_entry* entries;  
    int capacity;
    int length;      
} ht;


ht* ht_create(void);


void ht_destroy(ht* table);


void* ht_get(ht* table, const char* key);


const char* ht_set(ht* table, const char* key, void* value);


int ht_length(ht* table);


typedef struct
{
    const char* key;  
    void* value;     

    ht* _table;      
    int _index;
} hti;

hti ht_iterator(ht* table);

ht_next(hti* it);