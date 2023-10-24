// many odd sized allocations and interspersed frees
#include <assert.h>
#include <stdlib.h>
#include "p3Heap.h"

int main() {
   assert(init_heap(4096) == 0);
	void *ptr[9];
   ptr[0] = balloc(1);
   ptr[1] = (balloc(5));
   ptr[2] = (balloc(8));
   ptr[3] = (balloc(18));
   ptr[4] = (balloc(80));

   assert(ptr[0] != NULL);
   assert(ptr[1] != NULL);
   assert(ptr[2] != NULL);
   assert(ptr[3] != NULL);
   assert(ptr[4] != NULL);
   
   assert(bfree(ptr[1]) == 0);
   assert(bfree(ptr[0]) == 0);
   assert(bfree(ptr[3]) == 0);
   
   balloc(13);
   balloc(1);

   balloc(4);
   balloc(200);
   balloc(300);
   balloc(400);

   //assert(bfree(ptr[8]) == 0);
   //assert(bfree(ptr[6]) == 0);
   //assert(bfree(ptr[7]) == 0);

   balloc(400);

   exit(0);
}
