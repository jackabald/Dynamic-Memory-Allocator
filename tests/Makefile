C_FILES := $(wildcard *.c)
TARGETS := ${C_FILES:.c=}

all: ${TARGETS}

# Build each TARGET from its corresponding C source file
%: %.c
	gcc -I.. -g -m32 -Xlinker -rpath=.. -o $@ $< -L.. -lheap -std=gnu99

# Run the tests that only require allocating space on the heap
partA:
	./test101  #one block can be allocated
	./test102  #four blocks can be allocated (splitting must work)
	./test103  #alloc many different sized payloads
	./test104  #one block alloc, one fail
	./test105  #one block 2048, then 2047 fails
	./test110  #one alloc, then write to memory and check it is there
	./test121  #one alloc, confirm ptr returned is multiple of 8
	./test122  #four allocs confirm ptrs all multiple of 8
	./test123  #nine allocs random size, confirm each return is multiple of 8

# Run the tests that require allocating space on the heap and freeing works
partB:
	./test201  #two allocs, two frees, two allocs, 2 frees
	./test202  #many alloc/free cycles
	./test211  #8 allocs, 4 frees, more allocs/free cycles
	./test212  #4 allocs, 4 frees, one best fit, 
	./test213  #8 allocs, one fail, 3 frees, 1 fits now
	./test214  #5 allocs, 3 frees, check alloc(13) < alloc(80), alloc(1) < ptr[1]

# Run the tests that require allocating, freeing, and immediate coalescing  works
partC:
	./test301  #4 allocs, fill with allocs, alloc fail, free adj allocs, try again, alloc succeeds if free has implemented immediate coalescing
	./test302  #4 allocs, fill with allocs, free three adjacent blocks, alloc fails if no immediate coalesce
	./test303  #5 allocs, fill with allocs, free three adj blocks, alloc succeeds

partD:  #immediate coalesce with next and prev
	.test401  #4 allocs, fill with allocs, free immed coal with prev, alloc succeeds
	.test402  #fill with allocs, one free, alloc fail, adj free, alloc success

# Remove all generated target files (executables)
# Use before running make to get a clean re-build of all targets.
clean:
	rm -rf ${TARGETS} *.o

