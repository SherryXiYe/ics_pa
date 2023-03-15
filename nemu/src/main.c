int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);       // 进入用户界面主循环，输出NEMU的命令提示符:(nemu)。 in nemu/src/monitor/debug/ui.c

  return 0;
}
