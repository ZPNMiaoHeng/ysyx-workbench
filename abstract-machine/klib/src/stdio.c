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
  char c;
  char *s, *ob = out, *int_s;
  int size=0;

  memset(out, '\0', strlen(out));
  
  va_start(ap, fmt);
    while ((c=*fmt++)) {
      if(c != '%') {
        *ob++ = c;
      } 
      else {
        c = *(fmt++);
        switch (c) {
          case 's':
            s = va_arg(ap, char *);
            strcpy(ob, s);
            ob = ob + strlen(s);
            size = strlen(out);
            break;

          case 'd':
            d = va_arg(ap, int);
            int_s = itoa(d, ob);
            strcpy(ob, int_s);
            ob = ob + strlen(int_s);
            break;
          
          // case 'c':
          //   c = (char) va_arg(ap, int);
          //   break;
          }
        }
      }

  va_end(ap);
  return size;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
