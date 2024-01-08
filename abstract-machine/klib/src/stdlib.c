#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

static int
mini_itoa(long value, unsigned int radix, int uppercase, int unsig,char *buffer) {

  char *pbuffer = buffer;
  int negative = 0;
  int i, len;

  if (radix > 16) return 0;

  if (value < 0 && !unsig) {
    negative = 1;
    value = -value;
  }

  do {
    int digit = value % radix;
    *(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
    value /= radix;
  } while (value > 0);

  if (negative)
    *(pbuffer++) = '-';

  *(pbuffer) = '\0';

  len = (pbuffer - buffer);
  for (i = 0; i < len / 2; i++) {
    char j = buffer[i];
    buffer[i] = buffer[len-i-1];
    buffer[len-i-1] = j;
  }

  return len;
}

void itoa(const int n, char *buf) {
  mini_itoa(n, 10, 1, 0, buf);
}

static char *hbrk;
static void malloc_reset() {
  hbrk = (void *)ROUNDUP(heap.start, 8);
}


void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()

// #if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  // panic("Not implemented");
// #endif
  malloc_reset();
  size = (size_t)ROUNDUP(size, 8);
  char *old = hbrk;              // FIXME - hbrk应该是内存指针
  hbrk += size;
  assert((uintptr_t)heap.start <= (uintptr_t)hbrk && (uintptr_t)hbrk < (uintptr_t)heap.end);
  for (uint64_t *p = (uint64_t *)old; p != (uint64_t *)hbrk; p ++) {
    *p = 0;
  }
  // Assert((uintptr_t)hbrk - (uintptr_t)heap.start <= setting->mlim);
  return old;
  
  // return NULL;
}

void free(void *ptr) {
}

#endif
