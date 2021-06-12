// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
int numfreepages=0;
//uint pgrefcount[PHYSTOP >> PGSHIFT]; //20163081

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  uint pgrefcount[PHYSTOP >> PGSHIFT]; //20163081
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE){
    kmem.pgrefcount[V2P(p) >> PGSHIFT] = 0; //20163081
    kfree(p);
  }
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  //memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  if(kmem.pgrefcount[V2P(v)>>PGSHIFT]>0){ //20163081  
    kmem.pgrefcount[V2P(v)>>PGSHIFT] -= 1; //20163081
  }
  if(kmem.pgrefcount[V2P(v)>>PGSHIFT]==0){ //20163081
    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);
    numfreepages++;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  numfreepages--;
  r = kmem.freelist;
  kmem.pgrefcount[V2P((char*)r) >> PGSHIFT] = 1; //20163081
  if(r){
    kmem.freelist = r->next;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

int freemem(){
	return numfreepages;
}

uint get_refcounter(uint pa){ //20163081
  if(kmem.use_lock)
    acquire(&kmem.lock);
  uint refCount = kmem.pgrefcount[pa>>PGSHIFT];
  if(kmem.use_lock)
    release(&kmem.lock);
  return refCount;
}

void dec_refcounter(uint pa){ //20163081
  if(kmem.use_lock)
    acquire(&kmem.lock);
  if(kmem.pgrefcount[pa>>PGSHIFT] > 0){
    kmem.pgrefcount[pa>>PGSHIFT] -= 1; 
  }
  else{
   kmem.pgrefcount[pa>>PGSHIFT] = 0;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

void inc_refcounter(uint pa){ //20163081
  if(kmem.use_lock)
    acquire(&kmem.lock);
  kmem.pgrefcount[pa>>PGSHIFT] += 1;
  if(kmem.use_lock)
    release(&kmem.lock);
}