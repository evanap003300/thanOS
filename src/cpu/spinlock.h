#pragma once 
#include <stdint.h>

struct Spinlock {
    volatile bool locked = false;

    static inline uint64_t save_and_disable_interrupts() {
        uint64_t flags;
        __asm__ volatile ("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
        return flags;
    }

    static inline void restore_interrupts(uint64_t flags) {
        __asm__ volatile ("pushq %0; popfq" :: "r"(flags) : "memory", "cc");
    }

    void acquire() {
        while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE)) {
            __builtin_ia32_pause();
        }
    }

    void release() {
        __atomic_clear(&locked, __ATOMIC_RELEASE);
    }

    void release_irq(uint64_t flags) {
        release();
        restore_interrupts(flags);
    }

    uint64_t acquire_irq() {
        uint64_t flags = save_and_disable_interrupts();
        acquire();
        return flags;
    }
};