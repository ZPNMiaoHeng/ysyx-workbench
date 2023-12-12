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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

//*------------------------------
// TODO：Bug， 数据超出类型范围
// warning: integer overflow in expression of type ‘int’ results in ‘-2138594836’ [-Woverflow]
//*------------------------------

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static char *pa;
int divide_zero;

uint32_t choose(uint32_t n) {
  return rand() % n;
}

uint32_t gen_rand_op() {
  char* op;
  int rand = 0;
  // counter_expr_gen++;
  switch (choose(10)) {
    case 0: op = "+"; rand=3; break;
    case 1: op = "-"; rand=3; break;
    case 2: op = "*"; rand=3; break;

    // TODO - 等于判断
    case 3: op = "<"; rand=3; break;
    case 4: op = "<="; rand=3; break;
    case 5: op = ">"; rand=3; break;
    case 6: op = ">="; rand=3; break;
    
    // case 7: op = "=="; rand=3; break;
    // case 8: op = "!="; rand=3; break;
    // case 9: op = ">="; rand=3; break;

    // TODO - 位操作

    default: op = "/"; rand = 1; break;
  }
  // int buf_size = sprintf(pa++, "%s", op);
  int buf_size = sprintf(pa, "%s", op);
  pa = pa + buf_size;
  // printf("%d\n", buf_size);

  return rand;
}

static void gen_num(int divide_zero) {
  uint32_t num= rand() % 100;
  if((divide_zero == 1) && (num == 0)) {   // divide zero detect
    num = rand() % 100 + 1;
  }
  sprintf(pa++, "%d", num);
}

static void gen(int n) {
  sprintf(pa++, "%c", n);
}


static void gen_rand_expr(int i) {
  switch (choose(i)) {
    case 0: gen_num(divide_zero); break;
    case 1: gen('('); gen_rand_expr(3); gen(')'); break;
    default: gen_rand_expr(3); divide_zero = gen_rand_op(); gen_rand_expr(divide_zero); break;  // 当检测到除法时，后续生成数字
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    pa = buf;
    gen_rand_expr(3);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%d %s\n", result, buf);
  }
  return 0;
}
