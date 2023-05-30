#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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

// 输入c之后，NEMU开始进入指令执行的主循环cpu_exec()
static int cmd_c(char *args) {    
  cpu_exec(-1);     // in nemu/src/monitor/cpu-exec.c
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single-step execution. Pause execution after N instructions are executed in one step.", cmd_si },
  { "info", "Print program status. r: Print register status. w: Print watc point information.", cmd_info },
  { "x", "Scan memory. Take the value of the expression as the starting memory address, and output consecutive N 4 bytes in hexadecimal format.", cmd_x},
  { "p", "Expression evaluation.", cmd_p},
  { "w", "Set up monitoring points. Pauses program execution when the value of the expression (first parameter) changes.", cmd_w },
  { "d", "Delete the watch point with sequence number N.", cmd_d },
  /* TODO: Add more commands */

};

// NR_CMD:命令数
#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");          //读取第一个参数
  int i;

  if (arg == NULL) {
    /* no argument given  输出全部命令及其描述 */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {       // 查找参数对应的命令及其描述，并输出 
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  char *arg = strtok(NULL, " ");          //读取第一个参数
  uint64_t N = 1;        // N缺省为1，默认单步执行一条指令 
  if (arg != NULL) {
    sscanf(arg,"%ld",&N);
  }
  cpu_exec(N);
  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " "); 
  if(strcmp(arg,"r") == 0){     // info r:打印寄存器状态
    for(int i=0;i<8;i++){
      printf("%s\t%08x\t%u\n",reg_name(i,4),reg_l(i),reg_l(i));
    }
    //打印CR0和CR3寄存器
    printf("CR0=0x%x, CR3=0x%x\n",cpu.cr0,cpu.cr3);
  }else if(strcmp(arg,"w") == 0){   // info w：打印监视点信息
    print_wp_info();
  }else{
    printf("Bad command parameter. Please re-enter.\n");
  }
  return 0;
}

static int cmd_x(char *args){     // x N EXPR
  char *arg1 = strtok(NULL, " "); 
  int N;
  sscanf(arg1,"%d",&N);
  char *arg2 = arg1 + strlen(arg1) + 1;      // 第二个参数
  bool success = true;
  uint32_t addr = expr(arg2,&success);
  if(!success){
    printf("Wrong expression. Please re-enter.\n");
    return 0;
  }
  printf("0x%08x:    ",addr);
  for(int i=0;i<N;i++){
    printf("0x%08x  ",vaddr_read(addr,4));
    addr+=4;
  }
  printf("\n");
  return 0;
}

static int cmd_p(char *args){
  bool success = true;
  uint32_t result = expr(args, &success);
  if(!success){
    printf("Wrong expression. Please re-enter.\n");
  }else{
    printf("0x%08x\t%u\n",result,result);
  }
  return 0;
}

static int cmd_w(char *args){
  bool success = true;
  uint32_t result = expr(args, &success);
  if(!success){
    printf("Wrong expression. Please re-enter.\n");
    return 0;
  }
  WP* newWp = new_wp();
  if(newWp!=NULL){
    newWp->value = result;
    strcpy(newWp->expr,args);
    printf("watchpoint %d: %s  %d\n",newWp->NO,newWp->expr,newWp->value);
  }
  return 0;
}

static int cmd_d(char *args){
  char *arg = strtok(NULL, " "); 
  int N;
  sscanf(arg,"%d",&N);
  free_wp(N);
  return 0;
}

// 进入用户界面主循环，输出NEMU的命令提示符:(nemu)。
void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {        // 查找对应的命令，并传入参数
        if (cmd_table[i].handler(args) < 0) { return; }       // cmd_q返回-1，小于0退出
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
