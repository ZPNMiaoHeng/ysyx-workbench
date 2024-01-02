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
  static char NUM_CHAR[] = "0123456789ABCDEF";
  int len = 0;
  char buf[128];
  int buf_len = 0;
  while(*fmt != '\0' && len < n){
      switch(*fmt) {
          case '%':
            fmt++;
            switch(*fmt) {
              case 'd':
                int val = va_arg(ap, int);
                if(val == 0)
                  out[len++] = '0';
                if(val < 0) {
                  out[len++] = '-';
                  val = 0 - val;
                }
                for(buf_len = 0; val /= 10; buf_len++) {
                  buf[buf_len] = NUM_CHAR(val % 10);
                }

                for(int i = buf_len - 1; i >= 0; i--) {
                  out[len++] = buf[i];
                }

                break;
              case 'u':
                uint32_t uval = va_arg(ap, uint32_t);
                for(buf_len = 0; uval /= 10; buf_len++) {
                  buf[buf_len] = NUM_CHAR(uval % 10);
                }

                for(int i = buf_len - 1; i >= 0; i--) {
                  out[len++] = buf[i];
                }

                break;
              case 'c':
                char c = (char)va_arg(ap, int);    //va_arg函数没有char这个参数
                out[len++] = c;
                break;
              case 's':
                char *s = va_arg(ap, char*);
                for(int i = 0; s[i] != '\0'; i++)
                  out[len++] = s[i];
                break;
              case 'p':
                out[len++] = '0'; out[len++] = 'x';
                uint32_t address = va_arg(ap, uint32_t);
                for(buf_len = 0; address; address /= 16, buf_len++)
                  buf[buf_len] = NUM_CHAR[address % 16];
                for(int i = buf_len - 1; i >= 0; i--)
                  out[len++] = buf[i];
              break;               
            }
            break; // case % 的break.
          case '\n':
            out[len++] = '\n';
            break;
          default:
            out[len++] = *fmt;
      }
      fmt++;
  }
  out[len] = '\0';
  return len;
}

#endif
