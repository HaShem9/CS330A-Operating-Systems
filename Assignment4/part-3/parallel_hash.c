// Vaibhav Thakkar
// 170778

#include "common.h"
#include <pthread.h> 

/*Function templates. TODO*/
/*
  Return value is as follows,
   0 ==> result is zero
   1 ==> result is positive
   -1 ==> result is negative 
*/
static int atomic_add(unsigned int *ptr, long val)
{
       int ret = 0;
       asm volatile( 
                     "lock add %%rsi, (%%rdi);"
                     "pushf;"
                     "pop %%rax;" 
                     "movl %%eax, %0;"
                     : "=r" (ret)
                     : 
                     : "memory", "rax"
                    );

     
     if(ret & 0x80)
               ret = -1;
     else if(ret & 0x40)
               ret = 0;
     else
               ret = 1;
     return ret;
}

void done_one(struct input_manager *in, int tnum)
{
   pthread_mutex_lock(&in->lock);
   printf("%ld\n", in->being_processed[tnum-1]->id);
   in->being_processed[tnum-1] = NULL; 
   pthread_cond_broadcast(&in->cond);
   pthread_mutex_unlock(&in->lock);
   return;
}

int checkCondition(op_t* being_processed[THREADS], unsigned long key, int tnum){
  for(unsigned int i=0;i<THREADS;i++){
    if(i == (tnum-1)) continue;
    if((being_processed[i] != NULL) && (being_processed[i]->key == key) && (being_processed[i]->id < being_processed[tnum-1]->id))
      return 0;
  }

  return 1;
}

int read_op(struct input_manager *in, op_t *op, int tnum)
{
  unsigned size = sizeof(op_t);

  pthread_mutex_lock(&in->lock);
  if(in->curr > in->data + in->size){
    pthread_mutex_unlock(&in->lock);
    return -1;
  }
  
  memcpy(op, in->curr, size - sizeof(unsigned long));  //Copy till data ptr     
  if(op->op_type == GET || op->op_type == DEL){
      in->curr += size - sizeof(op->datalen) - sizeof(op->data);
  }else if(op->op_type == PUT){
      in->curr += size - sizeof(op->data);
      op->data = in->curr;
      in->curr += op->datalen;
  }else{
      assert(0);
  }
  
  if(in->curr > in->data + in->size){
    pthread_mutex_unlock(&in->lock);
    return -1;
  }

  in->being_processed[tnum-1] = op;        
  while(checkCondition(in->being_processed, op->key, tnum)!=1){
    pthread_cond_wait(&in->cond, &in->lock);
  }
  pthread_mutex_unlock(&in->lock);

  return 0; 
}

int lookup(hash_t *h, op_t *op)
{
  unsigned ctr;
  unsigned hashval = hashfunc(op->key, h->table_size);
  hash_entry_t *entry = h->table + hashval;
  ctr = hashval;
  unsigned long curr_key;
  unsigned curr_id;
  unsigned datalen;
  char *data;
  pthread_mutex_lock(&entry->lock);
  curr_key = entry->key ;
  curr_id = entry->id ;
  datalen = entry->datalen;
  data = entry->data;
  pthread_mutex_unlock(&entry->lock);
  
  while(ctr != hashval - 1){
    if((curr_key || curr_id == (unsigned) -1) && curr_key != op->key){
      ctr = (ctr + 1) % h->table_size;
      entry = h->table + ctr; 
      pthread_mutex_lock(&entry->lock);
      curr_key = entry->key ;
      curr_id = entry->id ;
      datalen = entry->datalen;
      data = entry->data;
      pthread_mutex_unlock(&entry->lock);
    }
    else{
      break;
    }
  }
  if(curr_key == op->key){
    op->datalen = datalen;
    op->data = data;
    return 0;
  }

  return -1;
}

int insert_update(hash_t *h, op_t *op)
{
  unsigned ctr;
  unsigned hashval = hashfunc(op->key, h->table_size);
  hash_entry_t *entry = h->table + hashval;
  
  assert(h && h->used < h->table_size);
  assert(op && op->key);
  ctr = hashval;

  hash_entry_t* mark = NULL;
  while(ctr != hashval - 1){
    pthread_mutex_lock(&entry->lock);

    if((entry->key || entry->id == (unsigned) -1) && entry->key != op->key){
      if((entry->id == (unsigned) -1) && (mark == NULL)){
        mark = entry;
        // pthread_mutex_unlock(&entry->lock);  // Now don't unlock
      }
      else{
        pthread_mutex_unlock(&entry->lock);
      }
      ctr = (ctr + 1) % h->table_size;
      entry = h->table + ctr; 
    }
    else{
      // don't unlock
      break;
    }
  } 

  assert(ctr != hashval - 1);

  if(entry->key == op->key){  //It is an update
    if(mark!=NULL){
      pthread_mutex_unlock(&mark->lock);
    }
    entry->id = op->id;
    entry->datalen = op->datalen;
    entry->data = op->data;
    pthread_mutex_unlock(&entry->lock);
    return 0;
  }
  assert(!entry->key);   // Fresh insertion
  if(mark != NULL){
      pthread_mutex_unlock(&entry->lock);
      entry = mark;
  }
  
  entry->id = op->id;
  entry->datalen = op->datalen;
  entry->key = op->key;
  entry->data = op->data;
  pthread_mutex_unlock(&entry->lock);

  atomic_add(&h->used, 1);
  return 0;
}

int purge_key(hash_t *h, op_t *op)
{
  unsigned ctr;
  unsigned hashval = hashfunc(op->key, h->table_size);
  hash_entry_t *entry = h->table + hashval;

  ctr = hashval;
  while(ctr != hashval - 1){
    pthread_mutex_lock(&entry->lock);
    if((entry->key || entry->id == (unsigned) -1) && entry->key != op->key){
      pthread_mutex_unlock(&entry->lock);
      ctr = (ctr + 1) % h->table_size;
      entry = h->table + ctr; 
    }
    else{
      break;
    }
  }

  if(entry->key == op->key){  //Found. purge it
    entry->id = (unsigned) -1;  //Empty but deleted
    entry->key = 0;
    entry->datalen = 0;
    entry->data = NULL;
    pthread_mutex_unlock(&entry->lock);

    atomic_add(&h->used, -1);
    return 0;
  }

  pthread_mutex_unlock(&entry->lock);
  return -1;    // Bogus purge
}

