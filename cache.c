#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "cache.h"

struct cache_entry* alloc_entry(char* path, char* content_type, void* content, int content_length)
{
  struct cache_entry* cache_entry = malloc(sizeof(*cache_entry));
  cache_entry->path               = path;
  cache_entry->content_type       = content_type;
  cache_entry->content            = content;
  cache_entry->content_length     = content_length;

  return cache_entry;
}

void free_entry(struct cache_entry* entry)
{
  
  free(entry);
}

void dllist_insert_head(struct cache* cache, struct cache_entry* ce)
{
  //Insert at the head
  if (cache->head == NULL)
  {
    cache->head = cache->tail = ce;
    ce->prev = ce->next = NULL;
  }
  else
  {
    cache->head->prev = ce;
    ce->next = cache->head;
    ce->prev = NULL;
    cache->head = ce;
  }
}

void dllist_move_to_head(struct cache* cache, struct cache_entry* ce)
{
  if (ce != cache->head)
  {
    if(ce == cache->tail)
    {
      //Tail
      cache->tail = ce->prev;
      cache->tail->next = NULL;
    }
    else
    {
      //Niether head nor tail
      ce->prev->next = ce->next;
      ce->next->prev = ce->prev;
    }

    ce->next = cache->head;
    cache->head->prev = ce;
    ce->prev = NULL;
    cache->head = ce;
  }
}

struct cache_entry *dllist_remove_tail(struct cache* cache)
{
  struct cache_entry* oldtail = cache->tail;

  cache->tail = oldtail->prev;
  cache->tail->next = NULL;

  cache->size--;

  return oldtail;
}

/*
 * Create A New Cache
 *
 * max_size: max number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache* cache_create(int max_size, int hashsize)
{
  struct cache* cache = malloc(sizeof(*cache));
  cache->max_size = max_size;

  if (hashsize)
  {
    cache->index = hashtable_create(hashsize, NULL);
  }
  else
  {
    cache->index = hashtable_create(0, NULL);
  }

  return cache;
}

void cache_free(struct cache* cache)
{
  struct cache_entry* cur_entry = cache->head;

  hashtable_destroy(cache->index);

  while (cur_entry != NULL)
  {
    struct cache_entry* next_entry = cur_entry->next;

    free_entry(cur_entry);

    cur_entry = next_entry;
  }

  free(cache);
}

void cache_put(struct cache* cache, char* path, char* content_type, void* content, int content_length)
{
  struct cache_entry* cache_entry = alloc_entry(path, content_type, content, content_length);
  
  hashtable_put(cache->index, path, content);
  
  if (cache->max_size  == cache->size)
  {
    free_entry(dllist_remove_tail(cache));
    dllist_insert_head(cache, cache_entry);
  }
  else
  {
    dllist_insert_head(cache, cache_entry);
    cache->size++;
  }
}

struct cache_entry* cache_get(struct cache* cache, char* path)
{
  void* content_type = hashtable_get(cache->index, path);
  
  if (content_type == NULL){ return NULL;}
  
  struct cache_entry* ce = malloc(sizeof(ce));

  hashtable_get(cache->index, path);
}
