#pragma once

struct run
{
  struct run *next;
};

void mm_init();

unsigned long kalloc();
void kfree(unsigned long);

struct buddy
{
  unsigned long size;
  unsigned long *bitmap;
};

void buddy_init();
unsigned long buddy_alloc(unsigned long);
void buddy_free(unsigned long);

unsigned long alloc_pages(unsigned long);
unsigned long alloc_page();
void free_pages(unsigned long);
