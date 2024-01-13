#include "lispy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lval *builtin_eval(lenv *e, lval *v) {
  LASSERT(v, v->count == 1,
          "Function 'eval' passed too many arguments, Expect: %d, Got: %d", 1,
          v->count);

  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
          "Function 'eval' passed incorrect type, Expect: %s, Got: %s",
          get_type_name(LVAL_QEXPR), get_type_name(v->cell[0]->type));

  lval *x = lval_take(v, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *v) {
  for (int i = 0; i < v->count; i++) {
    LASSERT(v, v->cell[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type, Expect: %s, Got: %s",
            get_type_name(LVAL_QEXPR), get_type_name(v->cell[0]->type));
  }

  lval *x = lval_pop(v, 0);

  while (v->count) {
    x = lval_join(x, lval_pop(v, 0));
  }

  lval_del(v);
  return x;
}

lval *builtin_tail(lenv *e, lval *a) {
  LASSERT(a, a->count == 1,
          "Function 'tail' passed too many arguments, Expect: %d, Got: %d", 1,
          a->count);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'tail' passed incorrect type, Expect: %s, Got: %s",
          get_type_name(LVAL_QEXPR), get_type_name(a->cell[0]->type));
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'tail' passed invalid Q-Expression, Expect: Q-Expression "
          "with at least 1 element, Got: empty Q-Expression");

  lval *v = lval_take(a, 0);

  // only need to pop and delete first entry of the list
  lval_del(lval_pop(v, 0));

  return v;
}

lval *builtin_head(lenv *e, lval *a) {
  LASSERT(a, a->count == 1,
          "Function 'head' passed too many arguments, Expect: %d, Got: %d", 1,
          a->count);
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect type, Expect: %s, Got: %s",
          get_type_name(LVAL_QEXPR), get_type_name(a->cell[0]->type));
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'head' passed invalid Q-Expression, Expect: Q-Expression "
          "with at least 1 element, Got: empty Q-Expression");

  lval *v = lval_take(a, 0);
  while (v->count > 1) {
    // keep pop the second entry of the list until we only have one left
    lval_del(lval_pop(v, 1));
  }

  return v;
}

lval *builtin_list(lenv *e, lval *v) {
  v->type = LVAL_QEXPR;
  return v;
};

lval *builtin_op(lval *a, char *op) {
  // make sure all arguments is number(for now)
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_NUM,
            "Function '%s' passed incorrect type, Expect: %s, Got: %s", op,
            get_type_name(LVAL_NUM), get_type_name(a->cell[i]->type));
  }

  lval *x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  /* While there are still elements remaining */
  while (a->count > 0) {
    /* Pop the next element */
    lval *y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0) {
      x->num += y->num;
    }
    if (strcmp(op, "-") == 0) {
      x->num -= y->num;
    }
    if (strcmp(op, "*") == 0) {
      x->num *= y->num;
    }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        x = lval_err("Division By Zero!");
        break;
      }
      x->num /= y->num;
    }

    lval_del(y);
  }

  lval_del(a);
  return x;
}
lval *builtin_add(lenv *e, lval *v) { return builtin_op(v, "+"); }

lval *builtin_sub(lenv *e, lval *v) { return builtin_op(v, "-"); }

lval *builtin_mul(lenv *e, lval *v) { return builtin_op(v, "*"); }

lval *builtin_div(lenv *e, lval *v) { return builtin_op(v, "/"); }

void lenv_add_builtins(lenv *e) {
  /* List Functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "=", builtin_put);
  lenv_add_builtin(e, "exit", builtin_exit);
  lenv_add_builtin(e, "print_env", builtin_print_env);
  lenv_add_builtin(e, "\\", builtin_lambda);

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
}

lval *builtin_def(lenv *e, lval *v) { return builtin_var(e, v, "def"); }

lval *builtin_put(lenv *e, lval *v) { return builtin_var(e, v, "="); }

lval *builtin_var(lenv *e, lval *v, char *func) {
  LASSERT_TYPE(func, v, v->cell[0], LVAL_QEXPR, "passed incorrect type");

  lval *syms = v->cell[0];

  // make sure symbol list is all SYMBOL
  for (int i = 0; i < syms->count; i++) {
    LASSERT_TYPE(func, v, syms->cell[i], LVAL_SYM, "can not define non-symbol");
  }

  LASSERT(v, syms->count == v->count - 1,
          "Function '%s' cannot define incorrect "
          "number of values to symbols",
          func);

  for (int i = 0; i < syms->count; i++) {
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], v->cell[i + 1]);
    }
    if (strcmp(func, "=") == 0) {
      lenv_put(e, syms->cell[i], v->cell[i + 1]);
    }
  }

  lval_del(v);
  return lval_sexpr();
}

lval *builtin_print_env(lenv *e, lval *v) {
  for (int i = 0; i < e->count; i++) {
    printf("%s: ", e->syms[i]);
    lval_print(e, e->vals[i]);
    putchar('\n');
  }

  lval_del(v);
  return lval_sexpr();
}

lval *builtin_exit(lenv *e, lval *v) {
  LASSERT(v, v->count == 1,
          "Function 'exit' passed too many arguments, Expect: %d, Got: %d", 1,
          v->count);
  LASSERT(v, v->cell[0]->type == LVAL_NUM,
          "Function 'head' passed incorrect type, Expect: %s, Got: %s",
          get_type_name(LVAL_NUM), get_type_name(v->cell[0]->type));

  printf("exit with code %ld\n", v->num);

  exit(v->num);
}

lval *builtin_lambda(lenv *e, lval *v) {
  LASSERT_COUNT("\\", v, v, 2, "passed incorrect number of elements");
  LASSERT_TYPE("\\", v, v->cell[0], LVAL_QEXPR,
               "first element should be a Q-Expression");
  LASSERT_TYPE("\\", v, v->cell[1], LVAL_QEXPR,
               "second element should be a Q-Expression");

  // make sure argument list only contain symbols
  for (int i = 0; i < v->cell[0]->count; i++) {
    LASSERT_TYPE("\\", v, v->cell[0]->cell[i], LVAL_SYM,
                 "can not define non Symbol");
  }

  lval *formals = lval_pop(v, 0);
  lval *body = lval_pop(v, 0);
  lval_del(v);

  return lval_lambda(formals, body);
}
