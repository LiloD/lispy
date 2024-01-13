#include "lispy.h"
#include <stdio.h>

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

// ToDo: refactor later
void lval_print_func(lenv *e, lval *v) {
  // for (int i = 0; i < e->count; i++) {
  // if (e->vals[i]->type == LVAL_FUNC && e->vals[i]->func == v->func) {
  // printf("<function: %s>", e->syms[i]);
  // return;
  //}
  //}
  //
  // printf("<function: anonymous>");

  printf("<function>");
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
