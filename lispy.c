#include "lispy.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// bfs through the ast
traverse_stat bfs(mpc_ast_t *t) {
  traverse_stat stat;
  stat.max_children = 0;
  stat.num_of_branches = 0;
  stat.num_of_leaves = 0;
  stat.num_of_nodes = 0;

  if (t == NULL) {
    return stat;
  }

  stat.num_of_nodes = 1;
  stat.num_of_branches = t->children_num;
  stat.max_children = t->children_num;

  // because this node has no children, so it's a leaf
  if (t->children_num == 0) {
    stat.num_of_leaves = 1;
    return stat;
  }

  for (int i = 0; i < t->children_num; i++) {
    traverse_stat child_stat = bfs(t->children[i]);

    // handle traverse stat from child
    // handle num of nodes
    stat.num_of_nodes += child_stat.num_of_nodes;

    // handle num of leaves
    stat.num_of_leaves += child_stat.num_of_leaves;

    // handle num of branch
    stat.num_of_branches += child_stat.num_of_branches;

    // handle max num of children
    if (child_stat.max_children > stat.max_children) {
      stat.max_children = child_stat.max_children;
    }
  }

  return stat;
}

lval *lval_read_num(mpc_ast_t *t) {
  errno = 0;
  long num = atol(t->contents);
  if (errno == ERANGE) {
    return lval_err("invalid number");
  }
  return lval_num(num);
}

lval *lval_read_sym(mpc_ast_t *t) { return lval_sym(t->contents); }

lval *lval_add(lval *v, lval *x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

/*
this is a ast example
>
  regex
  operator|char:1:1 '+'
  expr|number|regex:1:3 '1'
  expr|>
    char:1:5 '('
    operator|char:1:6 '*'
    expr|number|regex:1:8 '2'
    expr|number|regex:1:10 '3'
    char:1:11 ')'
  regex
*/
lval *lval_read(mpc_ast_t *t) {
  printf("read ast node: %s\n", t->tag);
  if (strstr(t->tag, "number")) {
    return lval_read_num(t);
  }
  if (strstr(t->tag, "symbol")) {
    return lval_read_sym(t);
  }

  lval *x = NULL;

  if (strcmp(t->tag, ">") == 0) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "sexpr")) {
    x = lval_sexpr();
  }

  // take care of children
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, ")") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

lval *lval_pop(lval *sexpr, int idx) {
  lval *v = sexpr->cell[idx];

  // shift the s-expr's memory of cell array
  memmove(&sexpr->cell[idx], &sexpr->cell[idx + 1],
          sizeof(lval *) * (sexpr->count - idx - 1));

  sexpr->count--;

  // reallocate the memory
  // cause we moved before and the total count decreased by 1
  sexpr->cell = realloc(sexpr->cell, sizeof(lval *) * sexpr->count);

  return v;
}

lval *lval_take(lval *sexpr, int idx) {
  lval *v = lval_pop(sexpr, idx);
  lval_del(sexpr);
  return v;
}

lval *lval_eval_sexpr(lval *v) {
  // evaluate every child
  // ATTENTION: the number of child is the same after eval
  // *only for this node*
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  // take care of error
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) {
      // take the ERR out of s-expr, and delete everything else
      // return lval_take(v, i);
      return lval_take(v, i);
    }
  }
  if (v->count == 0) {
    return v;
  }

  if (v->count == 1) {
    return lval_take(v, 0);
  }

  /* Ensure First Element is Symbol */
  lval *op = lval_pop(v, 0);
  if (op->type != LVAL_SYM) {
    lval_del(op);
    lval_del(v);
    return lval_err("S-expression Does not start with symbol!");
  }

  // the rest of children inside v is the arguments to op
  lval *result = builtin_op(v, op->sym);
  lval_del(op); // operator is useless now
  return result;
}

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

lval *lval_eval(lval *v) {
  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(v);
  }
  return v;
}

// some constructors
lval *lval_num(long x) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval *lval_sym(char *s) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval *lval_err(char *e) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(e) + 1);
  strcpy(v->err, e);
  return v;
}

lval *lval_sexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval *v) {
  switch (v->type) {
  case LVAL_NUM:
    break;
  case LVAL_SYM:
    free(v->sym);
    break;
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SEXPR:
    for (int i = 0; i < v->count; i++) {
      lval_del(v->cell[i]);
    }
    free(v->cell);
    break;
  }

  // dont forget to free itself
  free(v);
}

void lval_sexpr_print(lval *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval *v) {
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
  case LVAL_SEXPR:
    lval_sexpr_print(v, '(', ')');
    break;
  }
}

void lval_println(lval *v) {
  lval_print(v);
  putchar('\n');
}
