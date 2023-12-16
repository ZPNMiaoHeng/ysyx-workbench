#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  while ((*dst++ = *src++))
    ;
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  while (*dst) {
    dst++;
    while ((*dst++ = *src++))
      ;
  }
  return dst;
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
    *p++ = (unsigned char)c;
  }
  return s;
  // panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *p = out;
  const unsigned char *q=in;
  while (n--)
  {
    *p++ = *q++;
  }
  return out;
  // panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
