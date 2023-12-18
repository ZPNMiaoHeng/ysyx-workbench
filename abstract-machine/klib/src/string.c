#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i = 0;
  while(*s != '\0') {
    s++;
    i++;
  }
  return i;
  // panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  char *return_value = dst;
  while ((*dst++ = *src++))
    ;
  return return_value;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
    char *return_value = dst;
    while(*dst != '\0')
       dst++;
    strcpy(dst ,src);
    
    return return_value;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 == *s2; s1++, s2++) {
    if(*s1 == '\0')
      return 0;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  for(int i=0; i<n; i++) {
    *p = (unsigned char)c;
    p++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *p = out;
  const unsigned char *q=in;
  while (n--) {
    *p++ = *q++;
  }
  return 0;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p = s1, *q=s2;
  for(int i=0; i<n; i++) {
    if(*p != *q) {
      break;
    }
    if(i == n-1) {
      return 0;
    }
    p++;
    q++;
  }
  return *p - *q;
}

#endif
