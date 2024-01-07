#include "lispy.h"

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
          "Function 'tail' passed too many arguments, Expect: %s, Got: %s", 1,
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
          "Function 'head' passed too many arguments, Expect: %s, Got: %s", 1,
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

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
}

lval *builtin_def(lenv *e, lval *v) {
  LASSERT(v, v->cell[0]->type == LVAL_QEXPR,
          "Function 'def' passed incorrect type, Expect: %s, Got: %s",
          get_type_name(LVAL_QEXPR), get_type_name(v->cell[0]->type));

  lval *syms = v->cell[0];

  // make sure symbol list is all SYMBOL
  for (int i = 0; i < syms->count; i++) {
    LASSERT(v, syms->cell[i]->type == LVAL_SYM,
            "Function 'def' cannot define non-symbol");

    LASSERT(v, syms->cell[i]->type == LVAL_SYM,
            "Function 'def' passed incorrect type, Expect: %s, Got: %s",
            get_type_name(LVAL_SYM), get_type_name(syms->cell[i]->type));
  }

  LASSERT(v, syms->count == v->count - 1,
          "Function 'def' cannot define incorrect "
          "number of values to symbols");

  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->cell[i], v->cell[i + 1]);
  }

  lval_del(v);
  return lval_sexpr();
}
