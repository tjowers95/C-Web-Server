#ifndef _CACHE_H
#define _CACHE_H
// Individual Hashtable Entry
struct cache_entry
{
  char* path;    //Endpoint path -- key to the cache
  char* content_type;
  int content_length;
  void* content;

  struct cache_entry* prev,* next; //Doubly-linked list
};
//A Cache
struct cache
{
  struct hastable* index;
  struct cache_entry* head,* tail;  //Doubly-linked list
  int max_size; 		    //Max Number of Entries
  int size;			    //Current Number of Entries
};

extern struct cache_entry* alloc_entry(char* path, char* content_type, void* content, int content_length);
extern void free_entry(struct cache_entry* entry);
extern struct cache* cache_create(int max_size, int hashsize);
extern void cache_free(struct cache* cache);
extern void cache_put(struct cache* cache, char* path, char* content_type, void* content, int content_length);
extern struct cache_entry* cache_get(struct cache* cache, char* path);

#endif
