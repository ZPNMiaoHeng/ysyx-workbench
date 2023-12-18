#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d;
  // char c;
  char *s;
  // int return_value;

  va_start(ap, fmt);

  while(*fmt)
    switch (*fmt++) {
      case 's':
        memset(out, '\0', 128);
        s = va_arg(ap, char *);
        strcpy(out, s);
        // return_value = sizeof(strcpy(out, s));
        break;

      case 'd':
        memset(out, '\0', 128);
        d = va_arg(ap, int);   // 如何将d存入out中呢？？？
        *out = itoa(d);
        out++;
/*
      case 'c':
        c = (char) va_arg(ap, int);
        break;
    */
    }
  va_end(ap);

  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
