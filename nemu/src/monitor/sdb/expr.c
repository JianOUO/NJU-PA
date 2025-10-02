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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBER,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus \在C语言中是转义字符, \\+在编译后变为\+，匹配"+"的正则表达式
  {"==", TK_EQ},        // equal
  {"-*[0-9]+", TK_NUMBER},  // decimal number
  {"-", '-'},           // minus
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"\\(", '('},          // left parenthesis
  {"\\)", ')'},         // right parenthesis
};

#define NR_REGEX ARRLEN(rules)

// 编译后的正则表示式
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        // %.*s .* 表示精度由参数指定
        // 限制输出的字符数量
        // 如果精度大于字符串长度，输出整个字符串
        // 如果精度小于字符串长度，只输出前 N 个字符
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        Assert(nr_token < 32,"token number should less than 32");
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '+': {
            tokens[nr_token++].type = '+';
            break;
          };
          case TK_EQ: {
            tokens[nr_token++].type = TK_EQ;
            break;
          };
          case '-': {
            tokens[nr_token++].type = '-';
            break;
          };
          case '*': {
            tokens[nr_token++].type = '*';
            break;
          };
          case '/': {
            tokens[nr_token++].type = '/';
            break;
          };
          case '(': {
            tokens[nr_token++].type = '(';
            break;
          };
          case ')': {
            tokens[nr_token++].type = ')';
            break;
          }
          case TK_NUMBER: { 
            if (tokens[nr_token].str[0] == '-' && nr_token > 0) {
                int type = tokens[nr_token - 1].type;
                if (type == TK_NOTYPE) continue;
                if (type == TK_NUMBER || type == ')') {
                  tokens[nr_token++].type = '-';
                  substr_len--;
                  substr_start++;                  
                  break;
                }
            }
            tokens[nr_token].type = TK_NUMBER;
            Assert(substr_len < 32, "number lens should less than 32");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token++].str[substr_len] = '\0';
            break;
          }
          default: panic("Unkowned Token Type");
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      // %*,s * 表示字段宽度由参数指定
      // 如果字符串长度小于指定宽度，会在左侧填充空格
      // 如果字符串长度大于等于指定宽度，直接输出字符串
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool error = false;

static word_t string2number(char *str) {
  word_t val = 0;
  int minus_num = 0;
  int len = strlen(str);
  for (int i = 0; i < len; i++) {
    if (str[i] == '-')  minus_num++;
  }
  if (len - minus_num > 10) {
    Log("Expr error");
    error = true;
    printf("%s out of uint32 max\n", str);
    return 0;
  } else if (len == 10) {
    const char *max_uint = "4294967295";
      for (int i = 0; i < 10; i++) {
        if (str[i] < max_uint[i]) break;
        if (str[i] > max_uint[i]) {
          Log("Expr error");
          error = true;
          printf("%s out of uint32 max\n", str);
          return 0; 
        }
      }
  }
  if (sscanf(str + minus_num, "%u", &val) != 1) {
    Log("sscanf failed");
    error = true;
    return 0;
  }
  if (minus_num % 2) {
    if (val >= 0x80000000) {
      printf("%u can't be converted to negative number", val);
      error = true;
      return 0;
    } else {
      val = (word_t)(-1 * (int)val);
    }
  }
  return val;
}


static bool check_parentheses(int p, int q) {
  if (tokens[p].type == '(' && tokens[q].type == ')') {
    int top = 0;
    bool valid = true;
    int left[q - p];
    for (int i = p; i <= q; i++) {
      if (tokens[i].type == '(') {
        left[top++] = i;
      } else if (tokens[i].type == ')') {
        if (top <= 0) {
          Log("Expr error");
          error = true;
          return false;
        } else if (left[top - 1] == p && i != q) {
          valid = false;    
        }
        top--;
      }
    }
    if (top != 0) {
      Log("Expr error");
      error = true;
      return false;
    }
    return valid;
  } else {
    return false;
  }
}

static int find_mainop(int p, int q) {
  int top = 0;
  int main_op = p;
  for (int i = p; i <= q; i++) {
    switch (tokens[i].type) {
      case '(': {
        top++;
        break;
      };
      case ')': {
        top--;
        break;
      };
      case '+': {
        if (top) break;
        main_op = i;
        break;
      };
      case '-': {
        if (top || i == p) break;
        if (tokens[i - 1].type == TK_NUMBER || tokens[i - 1].type == ')') {
          main_op = i;
        }
        break;
      };
      case '*': {
        if (top || tokens[main_op].type == '+' || tokens[main_op].type == '-') {
          break;
        }
        main_op = i;
        break;
      };
      case '/': {
        if (top || tokens[main_op].type == '+' || tokens[main_op].type == '-') {
          break;
        }
        main_op = i;
        break;
      };
      default: break;
    }
  }
  if (main_op == p) {
    Log("Expr error");
    error = true;
    return 0;
  }
  return main_op;
}

static word_t eval(int p, int q) {
  if (p > q) {
    printf("Bad Expression\n");
    Log("Expr error");
    error = true;
    return 0;
  } else if (p == q) {
    return string2number(tokens[p].str);
  } else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  } else {
    if (error) return 0;
    int op = find_mainop(p, q);
    if (error) return 0;
    word_t val1 = eval(p, op - 1);
    if (error) return 0;
    word_t val2 = eval(op + 1, q);
    if (error) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2; 
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  word_t val = eval(0, nr_token - 1);
  if (error) *success = false;
  else *success = true;
  return val;
}
