#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(free_ == NULL){
    printf("Failed to set watch point! There are currently no idle watch points.\n");
    return NULL;
  }
  WP* freeWp =free_;
  free_ = free_->next;
  WP* htemp = head, *last=NULL;
  while (htemp != NULL){
    if( htemp->NO>freeWp->NO ){
      if(htemp==head){
        freeWp->next = head;
        head = freeWp;
      }else{
        last->next = freeWp;
        freeWp->next = htemp;
      }
      break;
    }
    last = htemp;
    htemp = htemp->next;
  }
  if(htemp==NULL){      //head链表为空或插到链表尾
    if(last==NULL){
      head = freeWp;
    }else{
      last->next = freeWp;
    }
    freeWp->next = htemp;
  }
  return freeWp; 
}

void free_wp(int num){
  if(head == NULL){
    printf("Failed to delete the watch point! There are currently no watch points in use.\n");
    return;
  }
  WP* p=head,*last=NULL;
  while (p != NULL){
    if(p->NO==num){
      if(p==head){
        head = p->next;
      }else{
        last->next = p->next; 
      }
      break;
    }
    last = p;
    p = p->next;
  }
  if(p==NULL){
    printf("Failed to delete the monitoring point! The monitoring point corresponding to this serial number is not in use.\n");
    return;
  }
  WP* temp = free_;
  last=NULL;
  while (temp != NULL){
    if( temp->NO>p->NO ){
      if(temp==free_){
        p->next = free_;
        free_ = p;
      }else{
        last->next = p;
        p->next = temp;
      }
      break;
    }
    last = temp;
    temp = temp->next;
  }
  if(temp==NULL){      //free_链表为空或插到链表尾
    if(last==NULL){
      free_ = p;
    }else{
      last->next = p;
    }
    p->next = temp;
  }
}

void print_wp_info(){
  WP* p=head;
  printf("Num\tWhat\t\t\tValue\t\tValue(HEX)\n");
  while (p!=NULL){
    printf("%d\t%-32s %-16d\t\t0x%08x\n",p->NO,p->expr,p->value,p->value);
    p=p->next;
  }
}

bool check_wp_changed(){
  bool changed = false; 
  WP* p=head;
  while (p!=NULL){
    bool success = true;
    uint32_t result = expr(p->expr, &success);
    if(!success){
      printf("Wrong expression. Please re-enter.\n");     //应该不会进入
      return 0;
    }
    if(p->value!=result){
      printf("Hardware watchpoint %d: %s\n",p->NO,p->expr);
      printf("Old value = %d\t0x%08x\n",p->value,p->value);
      printf("New value = %d\t0x%08x\n",result,result);
      changed = true;
      p->value = result;
    }
    p=p->next;
  }
  return changed;
}

