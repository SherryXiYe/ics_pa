#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[150];
  uint32_t value;
  /* TODO: Add more members if necessary */

} WP;

WP* new_wp();
void free_wp(int num);
void print_wp_info();
bool check_wp_changed();

#endif

