/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr = NULL;
    wp_pool[i].val = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(char *expression) {
  assert(expression);
  if (free_ == NULL) return NULL;
  bool success;
  free_->val = expr(expression, &success);
  if (!success) {
      printf("wp_scan: Expr compute failed. Expr: \"%s\"\n", expression);
      return NULL;
  }
  free_->expr = strdup(expression);
  assert(free_->expr);
  WP *tmp = free_->next;
  free_->next = head;
  head = free_;
  free_ = tmp;
  return head;
}

void free_wp(int number) {
  if (head == NULL) {
    printf("no used watchpoint\n");
    return;
  }
  if (number >= NR_WP) {
    printf("there is no NO.%d watchpoint \n", number);
    return;
  }
  WP *tmp;
  if (head->NO == number) {
    free(head->expr);
    head->expr = NULL;
    head->val = 0;
    tmp = head->next;
    head->next = free_;
    free_ = head;
    head = tmp;
  } else {
    tmp = head;
    while(tmp->next != NULL && tmp->next->NO != number) tmp = tmp->next;
    if (tmp->next == NULL) {
      printf("can't find the watchpoint\n");
      return;
    }
    WP *wp = tmp->next;
    free(wp->expr);
    wp->expr = NULL;
    wp->val = 0;
    tmp->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}

void wp_display() {
  WP *tmp = head;
  while (tmp != NULL) {
    printf("No.%d watchpoint, Expr: \"%s\", val: %u\n", tmp->NO, tmp->expr, tmp->val);
    tmp = tmp->next;
  }
}

void wp_scan() {
  WP *tmp = head;
  while (tmp != NULL) {
    assert(tmp->expr);
    bool success;
    word_t val = expr(tmp->expr, &success);
    if (!success) {
      printf("wp_scan: Expr compute failed. Expr: \"%s\"\n", tmp->expr);
      return;
    }
    if (val != tmp->val) {
      nemu_state.state = NEMU_STOP;
      printf("No.%d watchpoint: Expr \"%s\", val changed from %u to %u\n", tmp->NO, tmp->expr, tmp->val, val);
      tmp->val = val;
    }
    tmp = tmp->next;
  }
}
