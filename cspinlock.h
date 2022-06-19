// Reference: https://stackoverflow.com/questions/2744181/how-to-call-c-function-from-c
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef struct cspinlock cspinlock_t;

EXTERNC int cspin_lock(cspinlock_t *slock);
EXTERNC int cspin_trylock(cspinlock_t *slock);
EXTERNC int cspin_unlock(cspinlock_t *slock);
EXTERNC cspinlock_t* cspin_alloc();
EXTERNC void cspin_free(cspinlock_t* slock);

#undef EXTERNC


// typedef struct cspinlock cspinlock_t;

// //acquire the lock
// extern int cspin_lock(cspinlock_t *slock);

// //if the lock can not be acquired, return immediately
// extern int cspin_trylock(cspinlock_t *slock);

// //release the lock
// extern int cspin_unlock(cspinlock_t *slock);

// //allocate a lock
// extern cspinlock_t* cspin_alloc();

// //free a lock
// extern void cspin_free(cspinlock_t* slock);
