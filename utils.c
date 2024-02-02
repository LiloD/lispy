#include "lispy.h"
#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lval_sexpr_print(lenv *e, lval *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(e, v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lenv *e, lval *v) {
  switch (v->type) {
  case LVAL_NUM:
    printf("%li", v->num);
    break;
  case LVAL_ERR:
    printf("error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_STR:
    lval_print_str(v);
    break;
  case LVAL_FUNC:
    lval_print_func(e, v);
    break;
  case LVAL_SEXPR:
    lval_sexpr_print(e, v, '(', ')');
    break;
  case LVAL_QEXPR:
    lval_sexpr_print(e, v, '{', '}');
    break;
  }
}

void lval_print_func(lenv *e, lval *v) { printf("<function>"); }

void lval_print_str(lval *v) {
  char *escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);
  escaped = mpcf_escape(escaped);
  printf("\"%s\"", escaped);
  free(escaped);
}

void lval_println(lenv *e, lval *v) {
  lval_print(e, v);
  putchar('\n');
}
char *get_type_name(int t) {
  switch (t) {
  case LVAL_NUM:
    return "Number";
  case LVAL_ERR:
    return "Error";
  case LVAL_SYM:
    return "Symbol";
  case LVAL_STR:
    return "String";
  case LVAL_SEXPR:
    return "S-Expression";
  case LVAL_QEXPR:
    return "Q-Expression";
  case LVAL_FUNC:
    return "Function";
  default:
    return "Unknown";
  }
}
