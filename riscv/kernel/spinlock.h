// Mutual exclusion lock.
// 自旋锁, 没有抢夺到锁的进程将会一直尝试，简单地实现了互斥
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

