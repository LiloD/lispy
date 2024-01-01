#ifndef lispy_h
#define lispy_h
#include "mpc.h"

typedef struct traverse_stat {
  int num_of_nodes;
  int num_of_leaves;
  int num_of_branches;
  int max_children;
} traverse_stat;

typedef struct lval {
  int type; // value type
  long num;
  // error and symbol have some string data
  char *sym;
  char *err;
  // s-expr
  int count;
  struct lval **cell;
} lval;

// value type
enum {
  LVAL_NUM,
  LVAL_ERR,
  LVAL_SYM,
  LVAL_SEXPR, // S-expr is a variable list of other values
};

// error type
// enum {
// LERR_DIV_ZERO,
// LERR_BAD_OP,
// LERR_BAD_NUM,
//};

traverse_stat bfs(mpc_ast_t *t);

lval *lval_pop(lval *sexpr, int idx);
lval *lval_take(lval *sexpr, int idx);
lval *builtin_op(lval *v, char *op);
lval *lval_eval(lval *v);
lval *lval_eval_sexpr(lval *v);

// read and construct
lval *lval_read(mpc_ast_t *t);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read_sym(mpc_ast_t *t);
lval *lval_add(lval *v, lval *x);

lval *lval_num(long x);
lval *lval_err(char *e);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
void lval_del(lval *v);

// some utils
void lval_sexpr_print(lval *v, char open, char close);
void lval_print(lval *v);
void lval_println(lval *v);

#endif
