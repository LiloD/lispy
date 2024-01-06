#include "lispy.h"

lval *builtin_eval(lenv *e, lval *a) {
  LASSERT(a, a->count == 1,
          "Function 'eval' passed too many arguments, should only take one");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'eval' passed incorrect type, should be a Q-Expression");

  lval *v = lval_take(a, 0);
  v->type = LVAL_SEXPR;
  return lval_eval(e, v);
}

lval *builtin_join(lenv *e, lval *v) {
  for (int i = 0; i < v->count; i++) {
    LASSERT(v, v->cell[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type.");
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
          "Function 'tail' passed too many arguments, should only take one");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'tail' passed incorrect type, should be a Q-Expression");
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'tail' passed invalid Q-Expression, should not be an empty "
          "Q-Expression");

  lval *v = lval_take(a, 0);

  // only need to pop and delete first entry of the list
  lval_del(lval_pop(v, 0));

  return v;
}

lval *builtin_head(lenv *e, lval *a) {
  LASSERT(a, a->count == 1,
          "Function 'head' passed too many arguments, should only take one");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect type, should be a Q-Expression");
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'head' passed invalid Q-Expression, should not be an empty "
          "Q-Expression");

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
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a); // do not forget to delete it
      return lval_err("Cannot operate on non-number!");
    }
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

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
}
