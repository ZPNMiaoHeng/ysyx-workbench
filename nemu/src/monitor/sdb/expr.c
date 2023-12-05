/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/paddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_BEQ, TK_LEQ,
  TK_DIVIDE, TK_EXP,
  TK_NEGATIVE_NUBER,
  TK_LOGIC_AND,      // 逻辑与
  TK_LOGIC_OR,      // 逻辑或
  TK_POINTER         // 指针解引用
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},
  {">", '>'},
  {">=", TK_BEQ},
  {"<", '<'},
  {"<=", TK_LEQ},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"\\(", '('},
  {"\\)", ')'},
  {"\\*", '*'},         // mul or 指针解引用
  {"\\-", '-'},         // sub or 负数
  {"/", '/'},           // divide
  {"\\$", '$'},
  {"\\&", '&'},
  {"\\|", '|'},
  {"\\^", '^'},
  {"[0-9 | a-z | A-Z]*", TK_EXP},
  {"\\&&", TK_LOGIC_AND},
  {"\\||", TK_LOGIC_OR},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);  // rules.regex --compile--> store re[] array
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;  // match charater

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Assert(substr_len==0,);
        if(substr_len > 16) {
          printf("Input str is too long, Out of Array!!!\n");
          // TODO - Only use first str
          return false;
        }

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_EXP:                                                               //* 识别数字/字符
            tokens[nr_token].type = rules[i].token_type; 
            memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str)); 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++; break;
          
          case '+':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          
          case '*':
            if(nr_token == 0 || tokens[nr_token-1].type == '+' || tokens[nr_token-1].type == '-'|| tokens[nr_token-1].type == '*'|| tokens[nr_token-1].type == '/') {
              tokens[nr_token].type = TK_POINTER;
              // printf("Pointer !\n");
            } else {
              tokens[nr_token].type = rules[i].token_type;
              // printf("Mul\n");
            }
           nr_token++;
           break;
          case '-':
            if(nr_token == 0) {
              tokens[nr_token].type = rules[i].token_type;
              // printf("op - !\n");
            } else if(tokens[nr_token-1].type == '+' || tokens[nr_token-1].type == '-'|| tokens[nr_token-1].type == '*'|| tokens[nr_token-1].type == '/') {
              tokens[nr_token].type = TK_NEGATIVE_NUBER;
              // printf("neg !\n");
            } else {
              tokens[nr_token].type = rules[i].token_type;
              // printf("op - !\n");
            }
           nr_token++;
           break;
          case '$':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case '/':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case '(':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case ')':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case '>':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case TK_BEQ:    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case '<':    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case TK_LEQ:    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case TK_EQ:    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case TK_NEQ:    tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          case '&': 
            if(tokens[nr_token-1].type != '&') {
              tokens[nr_token].type = rules[i].token_type; nr_token++;
            } else {
              tokens[nr_token-1].type = TK_LOGIC_AND;
            } 
            break;
          case '|':
            if(tokens[nr_token-1].type != '|') {
              tokens[nr_token].type = rules[i].token_type; nr_token++;
            } else {
              tokens[nr_token-1].type = TK_LOGIC_OR;              
            }
            break;
          case '^': tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
  int i = 0, parentheses_num = 0;

  for(i = p; p <= q; i ++) {
    if(tokens[i].type == '(')
      parentheses_num++;
    else if(tokens[i].type == ')')
      parentheses_num--;
    
    if(parentheses_num == 0) break;
    else continue;
  }

  return (i == q);
}

int main_operation(int p, int q) {
  int mainOpIndex, mainOp_priority = 0, parentheses = 0, op_priority = 0;
  for(mainOpIndex = 0; p < q; p++) {
    switch (tokens[p].type)
    {
    case '(': parentheses++; break;
    case ')': parentheses--; break;
    // case ''; ;break;
    case '$':                       // NOTE- 不确定优先级
    case TK_POINTER:
    case TK_NEGATIVE_NUBER: op_priority = 2 ; break;  // "--1"    /* 我们不实现自减运算, 这里应该解释成 -(-1) = 1 */
    case '*':
    case '/': op_priority = 3; break;
    case '+':
    case '-': op_priority = 4; break;

    case '<': 
    case TK_BEQ: 
    case '>': 
    case TK_LEQ: op_priority = 6; break;
    
    case TK_EQ:
    case TK_NEQ: op_priority = 7; break;
    case '&': op_priority = 8; break;
    case '^': op_priority = 9; break;
    case '|': op_priority = 10; break;
    case TK_LOGIC_AND: op_priority = 11; break;
    case TK_LOGIC_OR: op_priority = 12; break;
    
    default: op_priority = 0; break;
    }

    if(parentheses == 0) {
      if( mainOp_priority <= op_priority) {
        mainOpIndex = p;
        mainOp_priority = op_priority;
      }
    } 
  }

  return mainOpIndex;
}

bool *success = (bool *)true;
int eval(int p,int q) {
  int op, op_type, val1, val2;//, translator_dex = 10;
  char *endptr;
  long int translator_number;

  if (p > q) {
    return 0;
  }
  else if (p == q) {
    if ((strncmp(tokens[p].str, "0b", 2) == 0) || (strncmp(tokens[p].str, "0B", 2) == 0)) {
      translator_number = strtol(tokens[p].str+2, &endptr, 2);  // NOTE - strtol处理二进制转换时需要跨过前缀0b
    } else {
      translator_number = strtol(tokens[p].str, &endptr, 0);
    }

    if(*endptr != '\0') {
      Log("转换失败：输入字符串不是一个有效的整数。未转换部分：%s\n", endptr);
      return 0;
    } else {
      return translator_number;
    }
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }

  else {
    op = main_operation(p, q);
    printf("%d\n", op);
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
    op_type = tokens[op].type;

    switch (op_type) {
      case TK_NEGATIVE_NUBER: return -val2; break;
      case TK_POINTER: return paddr_read(val2, 4); break;
      case '+': return val1 + val2; break;
      case '-': return val1 - val2; break;
      case '*': return val1 * val2; break;
      case '/': return val1 / val2; break;
      case '$' : return isa_reg_str2val(tokens[p+1].str, success); break;
      
      case '<': return val1 < val2; break;
      case TK_BEQ: return val1 >= val2; break;
      case '>': return val1 < val2; break;
      case TK_LEQ: return val1 <= val2; break;
      case TK_EQ: return val1 == val2; break;
      case TK_NEQ: return val1 != val2; break;
      case TK_LOGIC_AND: return val1 && val2; break;
      case TK_LOGIC_OR: return val1 || val2; break;
      case '&': return val1 & val2; break;
      case '^': return val1 ^ val2; break;
      case '|': return val1 | val2; break;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    success = (bool *)false;   // false is 1 of int, force to pointer of pointer. 
    return 0;
  }
  success = (bool *)true;
  return eval(0, nr_token-1);
}
