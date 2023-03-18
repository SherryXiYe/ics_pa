#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 0, 
  TK_NUM , TK_HEX, TK_REG,
  TK_LBRACKET , TK_RBRACKET,
  TK_MINUS , TK_DEREF,
  TK_EQ, TK_NOTEQ, TK_ADD, TK_SUB, TK_MUL, TK_DIV,
  TK_AND, TK_OR, TK_NOT,
  /* TODO: Add more token types */

};

enum {
  OP_LV0=0, //number,register
  OP_LV1=10, //()
  OP_LV2_1=21, //unary+,unary-
  OP_LV2_2=22, //deference*
  OP_LV3=30, //*,/,%
  OP_LV4=40, //+,-
  OP_LV6=60, //!
  OP_LV7=70, //==, !=
  OP_LV11=110, //&&
  OP_LV12=120, //||
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // plus
  {"-", TK_SUB},
  {"\\*", TK_MUL},
  {"\\/", TK_DIV},
  {"\\(", TK_LBRACKET},
  {"\\)", TK_RBRACKET},
  {"0[xX][0-9A-Fa-f]+", TK_HEX},
  {"[1-9][0-9]*|0", TK_NUM},
  {"\\$[eE]?[a-zA-Z]{2}", TK_REG},
  {"==", TK_EQ},         // equal
  {"!=",TK_NOTEQ},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!",TK_NOT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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
  int precedence;
} Token;

Token tokens[32];
int nr_token;

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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE){
          break;
        }
        if(substr_len>29){
          panic("tokens array's str buffer overflow");
        }
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case TK_NUM:
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            tokens[nr_token].precedence = OP_LV0;
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);    //去掉0x
            tokens[nr_token].str[substr_len-2] = '\0';
            tokens[nr_token].precedence = OP_LV0;
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);    //去掉$
            tokens[nr_token].str[substr_len-1] = '\0';
            char* p = tokens[nr_token].str;                 //统一为小写字母
            while (*p){
              *p = tolower((unsigned char)*p);
              p++;
            }
            tokens[nr_token].precedence = OP_LV0;
            break;
          case TK_LBRACKET:
          case TK_RBRACKET:
            tokens[nr_token].precedence = OP_LV1;
            break;
          case TK_ADD:
            // 前一个token是NUM、HEX、REG、）的是双目加；前一个token是 &&、||、！、==、!=、+、-、*、/、（ 的是单目加
            if(nr_token==0 || !(tokens[nr_token-1].type == TK_NUM || tokens[nr_token].type == TK_HEX || tokens[nr_token].type == TK_REG || tokens[nr_token].type == TK_RBRACKET )){
              tokens[nr_token].precedence = OP_LV2_1; 
            }else{
              tokens[nr_token].precedence = OP_LV4;
            }
            break;
          case TK_SUB:
            if(nr_token==0 || !(tokens[nr_token-1].type == TK_NUM || tokens[nr_token].type == TK_HEX || tokens[nr_token].type == TK_REG || tokens[nr_token].type == TK_RBRACKET )){
              tokens[nr_token].precedence = OP_LV2_1; 
              tokens[nr_token].type = TK_MINUS;
            }else{
              tokens[nr_token].precedence = OP_LV4;
            }
            break;
          case TK_DIV:
            tokens[nr_token].precedence = OP_LV3;
            break;
          case TK_MUL:
            if(nr_token==0 || !(tokens[nr_token-1].type == TK_NUM || tokens[nr_token].type == TK_HEX || tokens[nr_token].type == TK_REG || tokens[nr_token].type == TK_RBRACKET )){
              tokens[nr_token].precedence = OP_LV2_2; 
              tokens[nr_token].type = TK_DEREF;
            }else{
              tokens[nr_token].precedence = OP_LV3;
            }
            break;
          case TK_NOT:
            tokens[nr_token].precedence = OP_LV6;
            break;
          case TK_EQ:
          case TK_NOTEQ:
            tokens[nr_token].precedence = OP_LV7;
            break;
          case TK_AND:
            tokens[nr_token].precedence = OP_LV11;
            break;
          case TK_OR:
            tokens[nr_token].precedence = OP_LV12;
            break;
          default: TODO();
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool *success){
  int count =0;
  int first_rbracket = -1;
  bool isFirst=true;
  for(int i=p;i<=q;i++){
    if(tokens[i].type==TK_LBRACKET){
      count++;
    }else if(tokens[i].type == TK_RBRACKET){
      count--;
      if(isFirst){
        first_rbracket = i;
        isFirst = false;
      }
    }
    if(count<0)
      break;
  }
  if(count!=0){
    Log("Bracket mismatch.\n");
    *success = false;
    return false;
  }
  if(tokens[p].type!=TK_LBRACKET || first_rbracket != q){
    return false;
  }else{
    return true;
  }
} 

// 找到子表达式中优先级最低的token的位置（单目运算符+、-、*、！是右结合，其余是左结合）
int dominant_op_pos(int p, int q){
  int min_pos = p;
  for(int i=p+1;i<=q;i++){
    if(tokens[i].type==OP_LV2_1 || tokens[i].type==OP_LV2_2 || tokens[i].type == OP_LV6){
      if(tokens[i].precedence > tokens[min_pos].precedence)   //右结合，找最先出现的
        min_pos = i;
    }else{
      if(tokens[i].precedence >= tokens[min_pos].precedence)   //右结合，找最后出现的
        min_pos = i;
    }
  }
  return min_pos;
}

uint32_t eval(int p, int q, bool *success){
  if(!*success)
      return 0;
  if(p>q){
    Log("Bad expression.\n");
    *success = false;
    return 0;
  }else if(p==q){
    if(tokens[p].type == TK_NUM){
      uint32_t value;
      sscanf(tokens[p].str,"%d",&value);
      return value;
    }else if(tokens[p].type == TK_HEX){
      uint32_t value;
      sscanf(tokens[p].str,"%x",&value);
      return value;
    }else if(tokens[p].type == TK_REG){
      uint32_t value;
      if(get_reg_value(tokens[p].str,&value)){
        return value;
      }else{
        Log("Register name that does not exist.\n");
        *success = false;
        return 0;
      }
    }
  }else if(check_parentheses(p,q,success)==true){
    return eval(p+1,q+1,success);
  }else{
    int dominant_op = dominant_op_pos(p,q);
    if(dominant_op==q){
      Log("Bad dominant operator.\n");
      *success = false;
      return 0;
    }
    uint32_t right_val= eval(dominant_op+1,q,success);
    if(dominant_op==p){         //正常情况只能是单目运算符
      switch (tokens[dominant_op].type){
      case TK_ADD:
        return right_val;
      case TK_MINUS:
        return -right_val;
      case TK_NOT:
        return !right_val;
      case TK_DEREF:
        return vaddr_read(right_val,4);
      default:
        Log("Bad dominant operator.\n");
        *success = false;
        return 0;
      }
    }
    uint32_t left_val = eval(p,dominant_op-1,success);
    if(!*success)
      return 0;
    switch (tokens[dominant_op].type)     //处理双目运算符
    {
    case TK_ADD:
      return left_val + right_val;
    case TK_SUB:
      return left_val - right_val;
    case TK_MUL:
      return left_val * right_val;
    case TK_DIV:
      return left_val / right_val;
    case TK_EQ:
      return left_val == right_val;
    case TK_NOTEQ:
      return left_val != right_val;
    case TK_AND:
      return left_val && right_val;
    case TK_OR:
     return left_val || right_val;
    default:
      break;
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  
  return eval(0,nr_token-1,success);
}
