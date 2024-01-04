#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buf[5000];
  va_list ap;
  int ret = -1;
  va_start(ap, fmt);
  ret = vsprintf(buf, fmt, ap);
  va_end(ap);

  for (const char *p = buf; *p; p++) {
    putch(*p);
  }
  return ret ;
  // panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char ch, *s = out, *str, buf[128], digit[16], c;
  int num = 0, n;
  // word_t ln;
  memset(buf, 0, sizeof(buf));
  memset(digit, 0, sizeof(digit));
    
	while( (ch = *(fmt++))){
		if(ch != '%'){
			*s++ = ch;
      num++;
		}else{
			ch = *(fmt++);
			switch(ch){
				case 'd':
          n = va_arg(ap, int);
					if(n < 0){
						*s++ = '-';
						n = -n;
            num++;
					}
					itoa(n, buf);
          memcpy(s, buf, strlen(buf));
          s += strlen(buf);
          num += strlen(buf);
					break;
				case 's':
					str = va_arg(ap, char *);
					memcpy(s, str, strlen(str));
          s += strlen(str);
          num += strlen(str);
					break;
        case 'x':
          panic("vsprintf don't support! 'x'\n");
          /*
          n = va_arg(ap, int);
          xtoa(n, buf);
          memcpy(s, buf, strlen(buf));
          s += strlen(buf);
          num += strlen(buf);
          */
          break;
        case 'c':
          c = (char)va_arg(ap, int);
          // itoa(n, buf);
					// memcpy(s, buf, 1);
          *s ++ = c;
          num ++;
          break;
        case 'l':
          panic("vsprintf don't support! 'l'\n");
          // ch = *(fmt++);
          // switch (ch)
          // {
          // case 'u'://lu
          //   ln = va_arg(ap,word_t);
          //   // if(ln<0){
					// 	//   *s++ = '-';
					// 	//   n = -n;
          //   //   num++;
					//   // }
          //   litoa(ln, buf);
          //   memcpy(s, buf, strlen(buf));
          //   s += strlen(buf);
          //   num += strlen(buf);
          //   break;
          // case 'x'://lx
          //   // ln = va_arg(ap, word_t);
          //   // lxtoa(ln, buf);
          //   // memcpy(s, buf, strlen(buf));
          //   // s += strlen(buf);
          //   // num += strlen(buf);
          //   break;
          // default:
          //   panic("printf don't support!\n");
          //   break;
          // }
          // break;
				default:
          // putch(ch);
          *s++ = *fmt;
          break;
			}
		}
	} 
	*s = '\0';
  if(num > 0){
    return num;
  }
  return -1;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    int ret = -1;
    va_start(ap, fmt);
    ret = vsprintf(out, fmt, ap);
    va_end(ap);
    return ret;
/*
  va_list ap;
  int d;
  char c, buf[128];
  char *str, *ob = out;
  int size=0;

  memset(buf, '\0', sizeof(buf));
  
  va_start(ap, fmt);
    while ((c=*fmt++)) {
      if(c != '%') {
        *ob++ = c;
      } 
      else {
        c = *(fmt++);
        switch (c) {
          case 's':
            str = va_arg(ap, char *);
            memcpy(ob, str, strlen(str));
            ob = ob + strlen(str);
            size = strlen(str);
            break;

          case 'd':
            d = va_arg(ap, int);
            if(d < 0) {
              *ob++ = '-';
              d = -d;
            }
            itoa(d, buf);
            memcpy(ob, buf, strlen(buf));
            ob = ob + strlen(buf);
            size = strlen(buf);
            break;
          
          }
        }
      }
      *ob = '\0';
      size++; 

  va_end(ap);
  return size;
  */
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
