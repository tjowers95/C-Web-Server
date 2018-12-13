#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "llist.h"
#include "hashtable.h"

#define DEFAULT_SIZE 		128
#define DEFAULT_GROWTH_FACTOR	2

//Hash Table Entry
struct htent
{
  void* key;
  int key_size;
  int hashed_key;
  void* data;
};

//Used to Cleanup Linked Lists
struct foreach_callback_payload
{
  void* arg;
  void (*f)(void*, void*);
};

/*
 * Change Entry Count, Maintain Load Metrics
 */
void add_entry_count(struct hashtable* ht, int d)
{
  ht->num_entries += d;
  ht->load = (float)ht->num_entries / ht->size;
}

/*
 * Default Modulo Hashing Function
 */
int default_hashf(void* data, int data_size, int bucket_count)
{
  const int R = 31; //Small Prime
  int i = 0, h = 0;
  unsigned char* p = data;

  for (i; i < data_size; i++)
  {
    h = (R * h + p[i]) % bucket_count;
  }

  return h;
}

/*
 *Create New Hashtable
 */
struct hashtable* hashtable_create(int size, int (*hashf)(void*, int, int))
{
  if (size < 1)
  {
    size = DEFAULT_SIZE;
  }

  if (hashf == NULL)
  {
    hashf = default_hashf;
  }

  struct hashtable* ht = malloc(sizeof *ht);

  if (ht == NULL){ return NULL;}

  ht->size = size;
  ht->num_entries = 0;
  ht->load = 0;
  ht->bucket = malloc(size + sizeof(struct llist*));
  ht->hashf = hashf;

  int i = 0;
  for (i; i < size; i++)
  {
    ht->bucket[i] = llist_create();
  }

  return ht;
}

/*
 *Free An Entry
 */
void htent_free(void* htent, void* arg)
{
  (void)arg;
  free(htent);
}

/*
 * Destroy A Hashtable
 *
 * NOTE: Does not free the data pointer
 */
void hashtable_destroy(struct hashtable* ht)
{
  int i = 0;
  for (i; i < ht->size; i++)
  {
    struct llist* llist = ht->bucket[i];

    llist_foreach(llist, htent_free, NULL);
    llist_destroy(llist);
  }

  free(ht);
}

/*
 *Put In Hashtable
 */
void* hashtable_put(struct hashtable* ht, char* key, void* data)
{
  return hashtable_put_bin(ht, key, strlen(key), data);
}

/*
 * Put In Hashtable with Binary Key
 */
void* hashtable_put_bin(struct hashtable* ht, void* key, int key_size, void* data)
{
  int index = ht->hashf(key, key_size, ht->size);

  struct llist* llist = ht->bucket[index];

  struct htent* ent = malloc(sizeof *ent);
  ent->key = malloc(key_size);
  memcpy(ent->key, key, key_size);
  ent->key_size = key_size;
  ent->hashed_key = index;
  ent->data = data;

  if (llist_append(llist, ent) == NULL)
  {
    free(ent->key);
    free(ent);
    return NULL;
  }

  add_entry_count(ht, +1);

  return data;
}

/*
 *Comparison Function for Hashtable Entries
 */
int htcmp(void* a, void* b)
{
  struct htent* entA = a,* entB = b;

  int size_diff = entB->key_size - entA->key_size;

  if (size_diff)
  {
    return size_diff;
  }

  return memcmp(entA->key, entB->key, entA->key_size);
}

/*
 * Get from the Hashtable
 */
void* hashtable_get(struct hashtable* ht, char* key)
{
  return hashtable_get_bin(ht, key, strlen(key));
}

/*
 *Get from the Hashtable with Binary Data Key
 */
void* hashtable_get_bin(struct hashtable* ht, void* key, int key_size)
{
  int index = ht->hashf(key, key_size, ht->size);

  struct llist* llist = ht->bucket[index];

  struct htent cmpent;
  cmpent.key = key;
  cmpent.key_size = key_size;

  struct htent* n = llist_find(llist, &cmpent, htcmp);

  if (n == NULL){ return Null;}

  return n->data;
}

/*
 * Delete from the Hashtable
 */
void* hashtable_delete(struct hashtable* ht, char* key)
{
  return hashtable_delete_bin(ht, key, strlen(key));
}

/*
 * Delete from the Hashtable with Binary Key
 *
 * NOTE: Does not free the data -- just frees the Hashtable entry
 */

void* hashtable_delete_bin(struct hashtable* ht, void* key, int key_size)
{
  int index = ht->hasf(key, key_size, ht->size);

  struct llist* llist = ht->bucket[index];

  struct htent cmpent;
  cmpent.key = key;
  cmpent.key_size = key_size;

  struct htent* ent = llist_delete(llist, &cmpent, htcmp);

  if (ent == NULL){ return NULL;}

  void* data = ent->data;

  free(ent);
  
  add_entry_count(ht, -1);

  return data;
}

/*
 * Foreach callback function
 */
void foreach_callback(void* vent, void* vpayload)
{
  struct htent* ent = vent;
  struct foreach_callback_payload* payload = vpayload;

  payload->f(ent->data, payload->arg);
}

/*
 * Foreach element in the Hashtable
 *
 * NOTE: Elements returned in random order
 */
void hashtable_foreach(struct hashtable* ht, void (*f)(void*, void*), void* arg)
{
  struct foreach_callback_payload payload;

  payload.f = f;
  payload.arg = arg;

  int i = 0;
  for (i; i < ht->size; i++)
  {
    struct llist* llist = ht->bucket[i];

    llist_foreach(llist, foreach_callback, &payload);
  }
}

