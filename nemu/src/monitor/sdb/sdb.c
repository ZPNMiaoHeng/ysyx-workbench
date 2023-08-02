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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args){
  int step;
  char *token = strtok(args, " ");
  if(token == NULL) {
    step = 1;
  } else {
    step = atoi(args);
  }

  if(step == 0) {
    cpu_exec(1);
  } else if(step > 0 && step < 11) {
    cpu_exec(step);
  } else {
    Log("NEMU sdb only support 10 step!");
  }
  return 0;
}

static int cmd_info(char *args){
  isa_reg_display();
  return 0;
}

static int cmd_x(char *args){
  int step;
  paddr_t addr;
  sscanf(args,"%d %x", &step, &addr);
  if(addr < 0x80000000 || addr > 0x87ffffff) {
    printf(ANSI_FMT("Out of mem bound!!!\n", ANSI_FG_RED));
    return 0;
  }
  int i;
  for(i = 0; i < step; i++) {
    addr += sizeof(paddr_t);
    printf(ANSI_FMT("%#010x: " ,ANSI_FG_BLUE),addr);
    printf("%#010lx\n", paddr_read(addr, 4));
  }
  return 0;
}

static int cmd_p(char *args){
  bool *success = NULL;
  int result = expr(args, success);
  if(success == false) {
    printf("bad expr, please retry!\n");
  } else{
    printf("Result is %d\n", result);
  }
  return 0;
}

static int cmd_w(char *args){
  TODO();
  return 0;
}

static int cmd_d(char *args){
  TODO();
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);                          // pointer to function
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "si","si", cmd_si},
  { "i", "info", cmd_info},
  { "x","x", cmd_x},
  { "p","p", cmd_p},
  { "w","w", cmd_w},
  { "d","d", cmd_d},
  { "q", "Exit NEMU", cmd_q },

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;  // args command + '\0'
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
