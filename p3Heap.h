///////////////////////////////////////////////////////////////////////////////
// DO NOT EDIT or SUBMIT this file
// Copyright 2020-2023 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Fall 2023
///////////////////////////////////////////////////////////////////////////////

#ifndef __p3Heap_h
#define __p3Heap_h

int   init_heap(int sizeOfRegion);
void  disp_heap();

void* balloc(int size);
int   bfree(void *ptr);

void* malloc(size_t size) {
    return NULL;
}

#endif // __p3Heap_h__

