#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  // va_list ap;
  // va_start(ap, fmt);
  // char buffer[100];

  // int size = sprintf(buffer, *fmt, ...);
  panic("printf Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d;
  char c;
  // char *s, *ob = out;
  char *s, *ob = out, *int_s;
  int size=0;

  memset(out, '\0', strlen(out));
  
  va_start(ap, fmt);

  while ((c=*fmt++)) {
    if(c != '%') {
      *ob++ = c;
    } 
    else {
      c = *(fmt++);  //fmt 当前指向%，需要指向后面那个字符
      // c = *++fmt;
      switch (c) {
        case 's':
            s = va_arg(ap, char *);
            strcpy(ob, s);
            // memmove(ob, s, strlen(s));
            ob = ob + strlen(s);
            size = strlen(out);
            break;

        case 'd':
          d = va_arg(ap, int);
          // itoa(d, ob);
          int_s = itoa(d, ob);
          // strcpy(ob, int_s);
          memmove(ob, int_s, strlen(int_s));
          ob = ob + strlen(int_s);
          size = strlen(out);
          break;
          
        case 'c':   // TODO - test me ;
          c = (char) va_arg(ap, int);
          *ob++ = c;
          break;

        // case '%':

        // NOTE - "%%s"
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
