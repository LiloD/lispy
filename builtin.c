#include "lispy.h"
#include "mpc.h"
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
            get_type_name(LVAL_QEXPR), get_type_name(v->cell[i]->type));
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

  LASSERT(a, a->count != 0,
          "operator '%s' passed incorrect number of arguments, Expect: "
          "non-Zero, Got: 0",
          op);

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
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "global", builtin_def_global);
  lenv_add_builtin(e, ":=", builtin_def_local);
  lenv_add_builtin(e, "=", builtin_assign);
  lenv_add_builtin(e, "exit", builtin_exit);
  lenv_add_builtin(e, "print_env", builtin_print_env);
  lenv_add_builtin(e, "load", builtin_load);
  lenv_add_builtin(e, "print", builtin_print);
  lenv_add_builtin(e, "if", builtin_if);
  lenv_add_builtin(e, "\\", builtin_lambda);
  lenv_add_builtin(e, "==", builtin_eq);
  lenv_add_builtin(e, "!=", builtin_ne);

  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
}

lval *builtin_def_global(lenv *e, lval *v) {
  return builtin_var(e, v, "global");
}

lval *builtin_def_local(lenv *e, lval *v) { return builtin_var(e, v, ":="); }

lval *builtin_assign(lenv *e, lval *v) { return builtin_var(e, v, "="); }

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
    if (strcmp(func, "global") == 0) {
      lenv_put_global(e, syms->cell[i], v->cell[i + 1]);
    }
    if (strcmp(func, ":=") == 0) {
      lenv_put(e, syms->cell[i], v->cell[i + 1]);
    }
    if (strcmp(func, "=") == 0) {
      lval *err = lenv_assign(e, syms->cell[i], v->cell[i + 1]);
      if (err) {
        return err;
      }
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
  // LASSERT_COUNT("exit", v, v, 1, "passed incorrect number of elements");
  // LASSERT_TYPE("\\", v, v->cell[0], LVAL_NUM,
  //"first argument should be a number");

  if (v->num == 0) {
    printf("exit with code 0\n");
    exit(0);
  }

  lval *a = lval_take(v, 0);
  printf("exit with code %ld\n", a->num);

  exit(a->num);
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

lval *builtin_load(lenv *e, lval *v) {
  LASSERT_COUNT("load", v, v, 1, "passed incorrect number of elements");
  LASSERT_TYPE("load", v, v->cell[0], LVAL_STR,
               "first argument should be filename");

  mpc_result_t r;

  // printf("load `%s`", v->cell[0]->str);
  if (mpc_parse_contents(v->cell[0]->str, Lispy, &r)) {
    // mpc_ast_print(r.output);
    lval *expr = lval_read(r.output);
    while (expr->count) {
      lval *a = lval_pop(expr, 0);
      lval *x = lval_eval(e, a);
      if (x->type == LVAL_ERR) {
        lval_println(e, x);
      }
      lval_del(x);
    }
    lval_del(expr);
    lval_del(v);

    return lval_sexpr();
  } else {
    // handle parse error
    char *err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    lval *err = lval_err("Could not load library: %s", err_msg);
    free(err_msg);
    lval_del(v);

    return err;
  }
}

lval *builtin_print(lenv *e, lval *v) {
  for (int i = 0; i < v->count; i++) {
    lval_print(e, v->cell[i]);
    putchar(' ');
  }

  putchar('\n');
  lval_del(v);
  return lval_sexpr();
}

lval *builtin_if(lenv *e, lval *args) {
  LASSERT_GREAT_THAN("if", args, args, 1,
                     "passed incorrect number of elements");

  LASSERT_LESS_THAN("if", args, args, 4, "passed incorrect number of elements");

  // LASSERT_TYPE("if", args, args->cell[0], LVAL_NUM,
  //"1st arguments should be a number")

  for (int i = 1; i < args->count; i++) {
    LASSERT_TYPE("if", args, args->cell[i], LVAL_QEXPR,
                 "arguments should be a Q-Expression")
  }

  lval *cond = lval_pop(args, 0);
  lval *ret = NULL;

  // handle if condition is true
  if (!lval_is_zero(e, cond)) {
    ret = lval_take(args, 0);
  } else {
    // handle if condition is false
    if (args->count > 1) {
      ret = lval_take(args, 1);
    }
  }
  lval_del(cond);

  if (ret) {
    ret->type = LVAL_SEXPR;
  } else {
    ret = lval_sexpr();
  }

  return lval_eval(e, ret);
}
lval *builtin_cmp(lenv *e, lval *v, char *op) {
  LASSERT_COUNT(op, v, v, 2, "incorrect number of arguments");

  int r = 0;

  if (strcmp(op, "==") == 0) {
    r = lval_eq(e, v->cell[0], v->cell[1]);
    lval_del(v);
  } else if (strcmp(op, "!=") == 0) {
    r = !lval_eq(e, v->cell[0], v->cell[1]);
    lval_del(v);
  }
  return lval_num(r);
}

lval *builtin_eq(lenv *e, lval *v) { return builtin_cmp(e, v, "=="); }

lval *builtin_ne(lenv *e, lval *v) { return builtin_cmp(e, v, "!="); }
