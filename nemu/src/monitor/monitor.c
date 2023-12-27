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
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);
void init_ringbuf();

static void welcome() {
  // Log("Build Options:Debug: %s, Address sanitizer:%s", 
  //   MUXDEF(CONFIG_CC_DEBUG, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
  //   MUXDEF(CONFIG_CC_ASAN, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED))
  // );
  // Log("Trace: %s, Watchpoint:%s", 
  //   MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
  //   MUXDEF(CONFIG_WATCHPOINT_COND, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED))
  // );
  // Log("Build Options:\tDebug\tAddress sanitizer\tTrace\tWatchpoint\t");
  // Log("State:\t\t%s\t%s\t\t\t%s\t%s\t",
  Log("Debug\tAddress sanitizer\tTrace\tWatchpoint\tmtrace\tftrace\t");
  Log("%s\t\t%s\t\t\t%s\t%s\t\t%s\t%s\t",
    MUXDEF(CONFIG_CC_DEBUG, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
    MUXDEF(CONFIG_CC_ASAN, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
    MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
    MUXDEF(CONFIG_WATCHPOINT_COND, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
    MUXDEF(CONFIG_MTRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)),
    MUXDEF(CONFIG_FTRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED))
  );

  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>
#include <elf.h>
#include <cpu/decode.h>

void sdb_set_batch_mode();
word_t expr(char *e, bool *success);
void ftrace(Decode *s);

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *expr_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

typedef struct SYM_Func {
  char st_name[99];
  Elf32_Addr st_value;
  Elf32_Word st_size; 
} SYM_Func;

#define SYM_FUNC 32
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
SYM_Func sym_func[SYM_FUNC];
int func_num = 0, _start_index = 0;

static long test_expr() {
  if(expr_file == NULL) {
    Log("No gen-expr input is given.");
    return 0;
  }

  char buf[1024];
  char result[1024];
  bool *success = (bool *)true;
  int result_ret, buf_ret, expr_result;
  FILE *fp = fopen(expr_file, "r");
  Assert(fp, "Can not open '%s'", expr_file);
  
  while( (result_ret = fscanf(fp, "%s", result)) == 1) {
    buf_ret = fscanf(fp, "%s", buf);
    printf("Open test expr_file, %s, %s, %d, %d\n", result, buf, result_ret, buf_ret);
    
    expr_result = expr(buf, success);

    printf("%d\t%d\n", atoi(result), expr_result);
    assert(atoi(result) == expr_result);
  }
  
  fclose(fp);

  return 0;
}


static long load_elf() {
  if(elf_file == NULL) {
    Log("No elf is given.");
    return 0;
  }
  
  FILE    *fp;
  size_t  ret;
  unsigned char   buffer[4];
  
  char strtab_buf[1024], shstrtab_buf[1024];
  Elf32_Ehdr*     ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
  Elf32_Shdr  shdr[99];
  Elf32_Sym   sym[99];

  if (ehdr == NULL) {
      perror("malloc");
      return EXIT_FAILURE;
  }
  fp = fopen(elf_file, "rb");
  if (!fp) {
      perror("fopen");
      return EXIT_FAILURE;
  }
  Log("The elf file is %s", elf_file);
  
  ret = fread(buffer, sizeof(*buffer), ARRAY_SIZE(buffer), fp);
  if(ret != ARRAY_SIZE(buffer)) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }

  if((buffer[0] != 0x7f) | (buffer[1] != 'E') | (buffer[2] != 'L') | (buffer[3] != 'F')) {
    Assert(0, "ELF file magic is error! PASS ELF magic: %#04x%02x%02x%02x\n", buffer[0], buffer[1],
          buffer[2], buffer[3]);
  }

  // ELF Header
  // printf("\n");
  fseek(fp, 0, SEEK_SET);
  ret = fread(ehdr, sizeof(Elf32_Ehdr), 1, fp);
  if(ret != 1) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }
  // printf("ELF Header:\n");
  // printf("Magic:\t");
  // for(int i = 0; i < EI_NIDENT; i++) {
  //     // printf("%02x ", ehdr->e_ident[i]);
  //     if(i == EI_NIDENT-1) {
  //         // printf("\n");
  //     }
  // }
  if(ehdr->e_shnum > 99) {
      perror("ehdr->e_shnum is too small, please modify!");
      return EXIT_FAILURE;
  }

  // printf("\nSection Headers:\n");
  fseek(fp, ehdr->e_shoff, SEEK_SET);
  ret = fread(shdr, sizeof(Elf32_Shdr), ehdr->e_shnum, fp);
  if(ret != ehdr->e_shnum) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }
  
  fseek(fp, shdr[ehdr->e_shstrndx].sh_offset, SEEK_SET);
  ret = fread(shstrtab_buf, shdr[ehdr->e_shstrndx].sh_size, 1, fp);
  if(ret != 1) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }
  
  static int symtab_index = 0, symtab_entry = 0, strtab_index = 0;
  // printf("[Nr]\tName\t\t\tType\t\tAddr\t\tOff\tSize\tES\tFlg\tLk\tInf\tAl\n");
  for(int i = 0; i < ehdr->e_shnum; i++) {
      // printf("[%d]\t%-16s\t%-8d\t%-10x\t%x\t%x\t%d\t%d\t%d\t%d\t%d\t\n", 
      // i, &shstrtab_buf[shdr[i].sh_name], shdr[i].sh_type, shdr[i].sh_addr, shdr[i].sh_offset, shdr[i].sh_size, 
      // shdr[i].sh_entsize, shdr[i].sh_flags, shdr[i].sh_link, shdr[i].sh_info, shdr[i].sh_addralign);

      if(strcmp(&shstrtab_buf[shdr[i].sh_name], ".symtab") == 0) {
          symtab_index = i;
          // printf("Find shstrtab_buf:%d\t%x\n", i, shdr[i].sh_offset);
      } else if (strcmp(&shstrtab_buf[shdr[i].sh_name], ".strtab") == 0) {
          strtab_index = i;
          // printf("Find shstrtab_buf:%d\t%x\n", i, shdr[i].sh_offset);
      }
  }

  symtab_entry = shdr[symtab_index].sh_size / sizeof(Elf32_Sym);
  // printf("\nSymbol table '.symtab' contains %d entries:\n", symtab_entry);
  
  fseek(fp, shdr[strtab_index].sh_offset, SEEK_SET);
  ret = fread(strtab_buf, shdr[strtab_index].sh_size, 1, fp);
  if(ret != 1) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }
  
  fseek(fp, shdr[symtab_index].sh_offset, SEEK_SET);
  ret = fread(sym, sizeof(Elf32_Sym), symtab_entry, fp);
  if(ret != symtab_entry) {
      fprintf(stderr, "fread() failed: %zu\n", ret);
      exit(EXIT_FAILURE);
  }
  // printf("Num:\tValue\t\tSize\tType\tBind\tVis\tNdx\tName\n");
static  int j = 0;
  for(int i = 0; i < symtab_entry; i ++) {
      if((sym[i].st_info & 0xf) == STT_FUNC) {
          strcpy(sym_func[j].st_name, &strtab_buf[sym[i].st_name]);
          sym_func[j].st_value = sym[i].st_value;
          sym_func[j].st_size = sym[i].st_size;

          // printf("%-2d:\t%-8x\t%d\t%d\t%d\t%d\t%d\t%s\n", i,
          // sym[i].st_value, sym[i].st_size, sym[i].st_info &0xf, (sym[i].st_info>>4) &0x1, 
          // sym[i].st_other, sym[i].st_shndx, &strtab_buf[sym[i].st_name]);
          
          j ++;
      }
  }
  // printf("\n");
  func_num = j;

  for(int i = 0; i < func_num; i ++) {
    // printf("%-2d:\t%-8x\t%x\t%s\n", i,
    // sym_func[i].st_value, sym_func[i].st_size, sym_func[i].st_name);
    if(strcmp(sym_func[i].st_name, "_start") == 0) {
      _start_index = i;
      // printf("Find _start index:%d\n", _start_index);
    }
  }
  // printf("func_num = %d\n", func_num);
  free(ehdr);

  fclose(fp);

  return 0;

}

static int null_counter =0;
void ftrace(Decode *s) {
  int func_index_cur = _start_index, func_index_next = _start_index;

  for(int i = 0; i < func_num; i ++) {
    if((s->pc >= sym_func[i].st_value) && (s->pc < (sym_func[i].st_value + sym_func[i].st_size))) {
      func_index_cur = i;
    }
  }
  
  for(int j = 0; j < func_num; j ++) {
    if((s->dnpc >= sym_func[j].st_value) && (s->dnpc < (sym_func[j].st_value + sym_func[j].st_size))) {
      func_index_next = j;
    }
  }

  if(func_index_cur != func_index_next) {
    printf("%#x:", s->pc);
    if(s->dnpc == sym_func[func_index_next].st_value) {
      null_counter++;
      for(int i = 0; i < null_counter; i++) {
        printf(" ");
      }
      printf("call [%s@%#x]\n", sym_func[func_index_next].st_name, s->dnpc);
    } else {
      for(int i = 0; i < null_counter; i++) {
        printf(" ");
      }
      null_counter --;
      printf("ret [%s]\n", sym_func[func_index_cur].st_name);
    }
  }
// #endif

}

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"expr"     , required_argument, NULL, 'e'},
    {"elf"      , required_argument, NULL, 'f'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:f:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'e': expr_file = optarg; break;
      case 'f': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-e,--expr=FILE          test expr\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);            // log_file: stdout -> log_file

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);  // TODO - port ??

  /* Initialize the simple debugger. */
  init_sdb();

  init_ringbuf(); // TODO - Add to config
  
  /* Test expr */
  test_expr();

  /* elf!!*/
  load_elf();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
