


#define SBRK_ALIGN 8

static void *heap = 0;
extern int _HEAP_START;

void* _sbrk ( int increment ) {
 
    void* prevHeap;
    void* nextHeap;
    
    if (heap == 0) {
        // first allocation
        heap = (void*)&_HEAP_START;
    }

    prevHeap = heap;
    nextHeap = (void*)(((unsigned int)(heap + increment) + (SBRK_ALIGN-1)) & ~(SBRK_ALIGN-1));
    // TODO: verify
    heap = nextHeap;
    return (void*) prevHeap;
}
