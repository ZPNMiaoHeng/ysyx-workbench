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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};  // 监视点结构的池
static WP *head = NULL, *free_ = NULL;  // head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {                // 从free_链表中返回一个空闲的监视点结构
  if (free_ == NULL) {
    assert(0);
    return NULL;
  } else {
    WP* new_watchpoint = free_;
    free_ = free_->next;

    new_watchpoint->next = head;  //* 将new_watchpoint结点保存在head链表中
    head = new_watchpoint;

    return new_watchpoint;
  }
}

void free_wp(WP *wp) {      // ,free_wp()将wp从head中归还到free_链表中,
  wp->next = free_;
  free_ = wp;
  memset(wp->expr, '\0', sizeof(wp->expr));
  wp->old_value = 0;
}

void watchpoint_display() {
  // printf("Num\tType\tDisp\tEnb\tAddress\tWhat\n");   // gdb 完整版
  printf("Num\tWhat\told value\thead\n");
  for(WP *wp = head; wp; wp = wp->next) {
    printf("%d\t%s\t%#lx\n", wp->NO, wp->expr, wp->old_value);
  }

  // printf("Num\tWhat\told value\tfree\n");
  // for(WP *wp = free_; wp; wp = wp->next) {
  //   printf("%d\t%s\t%#lx\n", wp->NO, wp->expr, wp->old_value);
  // }
}

int find_wp(int NO) {
  WP *prev = NULL;
  WP *current = head;

  while (current != NULL) {
    if (current->NO == NO) {
      printf("Find free watchpoint:%d\t%s\t%#lx\n", current->NO, current->expr, current->old_value);
      if (prev == NULL) {
        head = current->next;
      } else {
        prev->next = current->next;
      }
      free_wp(current);
      return 0;
    }
    prev = current;
    current = current->next;
  }
  return 0;
}

bool watchpoint_checkout() {
  bool *success = (bool *)true;
  uint64_t new_result;
  int watchpoint_change = 0;

  for(WP *wp = head; wp; wp = wp->next) {
    // printf("%d\t%s\t%#lx\n", wp->NO, wp->expr, wp->old_value);
    new_result = expr(wp->expr, success);
    if(new_result != wp->old_value) {
      Log("Watchpoint %d: %s", wp->NO, wp->expr);
      printf("Watchpoint %d: %s\n\n", wp->NO, wp->expr);
      printf("Old value = %#lx\n", wp->old_value);
      printf("New value = %#lx\n", new_result);
      wp->old_value = new_result;
      watchpoint_change++;
    }
  }
  
  if(watchpoint_change != 0) {
      return false;
  } else {
      return true;
  }
}