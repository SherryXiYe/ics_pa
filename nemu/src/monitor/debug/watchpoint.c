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
    Log("Failed to set watch point! There are currently no idle watch points.\n");
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

}


