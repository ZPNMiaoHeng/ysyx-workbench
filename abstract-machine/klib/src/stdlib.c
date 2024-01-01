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

// * 实现简单十进制数字转换
char *itoa(int value, char *str) {
  char temp;
  int counter=0;
  char *return_ptr = str;

  do {
    counter++;
    *str++ = value % 10 + '0';
  } while((value /= 10) > 0);

  for(int i=0, j = counter-1; i<((j+1)/2); i++, j--) {
    temp = return_ptr[i];
    return_ptr[i] = return_ptr[j];
    return_ptr[j] = temp;
  }
  return return_ptr;
}

// void *itoa(int value, char *str) {
//   char temp;
//   int counter=0;
//   char *return_ptr = str;

//   do {
//     counter++;
//     *str++ = value % 10 + '0';
//   } while((value /= 10) > 0);

//   for(int i=0, j = counter-1; i<((j+1)/2); i++, j--) {
//     temp = return_ptr[i];
//     return_ptr[i] = return_ptr[j];
//     return_ptr[j] = temp;
//   }
//   return 0;
//   // return return_ptr;
// }


// void itoa(unsigned int n, char * buf) {
//   int i;
        
//   if(n < 10)  {
//     buf[0] = n + '0';
//     buf[1] = '\0';
//     return;
//   }
//   itoa(n / 10, buf);

//   for(i=0; buf[i]!='\0'; i++);
        
//   buf[i] = (n % 10) + '0';
        
//   buf[i+1] = '\0';
// }

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
