// Reference: https://rigtorp.se/spinlock/

#include "cspinlock.h"
#include <atomic>

typedef struct cspinlock{
    std::atomic<bool> lock_;
} cspinlock_t;

//acquire the lock
int cspin_lock(cspinlock_t *slock) {
    while (true) {
    if (!slock->lock_.exchange(true, std::memory_order_acquire)) {
        return 0; // acquire success
        // cspin_trylock and exchange will retrun false if success
    }
    while (slock->lock_.load(std::memory_order_relaxed));
    }
}

//if the lock can not be acquired, return immediately
int cspin_trylock(cspinlock_t *slock) {
    // if the frist is true, then the second will not be evaluate
    // if the first is false, the second step is to acquire the lock
    return !slock->lock_.load(std::memory_order_relaxed);
}

//release the lock
int cspin_unlock(cspinlock_t *slock) {
    slock->lock_.store(false, std::memory_order_release);
    return 0;
}

//allocate a lock
cspinlock_t* cspin_alloc() {
    cspinlock_t slock;
    slock.lock_ = false;
    return &slock;
}

// https://stackoverflow.com/questions/4134323/c-how-to-delete-a-structure
void cspin_free(cspinlock_t* slock) {
    // no need to free here?
    // free(slock);
    // slock = NULL;
    return;
}
