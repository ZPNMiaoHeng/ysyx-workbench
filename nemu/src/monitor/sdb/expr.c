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

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEM, TK_DIVIDE, TK_EXP,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus    // ??? why //+?? /+?
  {"==", TK_EQ},        // equal
  {"\\(", '('},
  {"\\)", ')'},
  {"\\*", '*'},         // mul
  {"\\-", '-'},         // sub
  {"/", '/'},           // divide
  {"\\$", '$'},
  {"[0]?[xhob]?[0-9]*", TK_NEM},   // number
  {"[0-9 | a-z | A-Z]*", TK_EXP}
  // {"[0-9]*", TK_NEM},   // number
  // {"\\(", '('},
  // {"\\)", ')'}

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

static Token tokens[32] __attribute__((used)) = {};
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

        if(substr_len > 8) {
          printf("Input str is too long, Out of Array!!!\n");
          // TODO - Only use first str
          return false;
        }

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NEM: tokens[nr_token].type = rules[i].token_type; strncpy(tokens[nr_token].str, substr_start, substr_len); nr_token++; printf("NUM!\n"); break;
          case '+': tokens[nr_token].type = rules[i].token_type; nr_token++; printf("+\n"); break;
          case '*': tokens[nr_token].type = rules[i].token_type; nr_token++; printf("*\n"); break;
          case '-': tokens[nr_token].type = rules[i].token_type; nr_token++; printf("-\n"); break;
          case '(': tokens[nr_token].type = rules[i].token_type; nr_token++; printf("-\n"); break;
          case ')': tokens[nr_token].type = rules[i].token_type; nr_token++; printf("-\n"); break;
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
// TODO - check_parentheses();
// TODO - main_operation();


int eval(int p,int q) {
  int op, op_type, val1, val2;
  if (p > q) {
    printf("bad expr\n");
    return 0;
  }
  else if (p == q) {
    printf("Single token\n");
    return atoi(tokens[p].str);

  // }
  // else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    //TODO - check_parentheses
    // return eval(p + 1, q - 1);
  }
  else {
    op = (p+q)/2; // the position of 主运算符 in the token expression;
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
    op_type = tokens[op].type;

    switch (op_type) {
      case '+': return val1 + val2; break;
      case '-': return val1 + val2; break;
      case '*': return val1 + val2; break;
      case '/': return val1 + val2; break;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    // *success = false;
    success = (bool *)false;
    return 0;
  }
    // success = (bool *)true;
  // return eval(0, nr_token-1);

  /* TODO: Insert codes to evaluate the expression. */
  // int i;
  // for(i = 0; i < 32; i++) {
    // printf("%d, %s\n", tokens[i].type, tokens[i].str);
  // }

  return 0;
}
