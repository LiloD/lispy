#ifndef lispy_h
#define lispy_h
#include "mpc.h"

// if condition not met, give an error and delete the args
#define LASSERT(args, cond, err)                                               \
  if (!(cond)) {                                                               \
    lval_del(args);                                                            \
    return lval_err(err);                                                      \
  }

typedef struct traverse_stat {
  int num_of_nodes;
  int num_of_leaves;
  int num_of_branches;
  int max_children;
} traverse_stat;

struct lenv;
struct lval;
typedef struct lenv lenv;
typedef struct lval lval;

typedef lval *(*lbuiltin)(lenv *, lval *);

typedef struct lval {
  int type; // value type
  long num;
  // error and symbol have some string data
  char *sym;
  char *err;
  lbuiltin func;
  // s-expr
  int count;
  struct lval **cell;
} lval;

typedef struct lenv {
  int count;
  char **syms;
  lval **vals;
} lenv;

// value type
enum {
  LVAL_NUM,
  LVAL_ERR,
  LVAL_SYM,
  LVAL_SEXPR, // S-expr is a variable list of other values
  LVAL_QEXPR,
  LVAL_FUNC,
};

traverse_stat bfs(mpc_ast_t *t);

// builtin
lval *builtin(lval *v, char *op);
lval *builtin_op(lval *v, char *op);
lval *builtin_add(lenv *e, lval *v);
lval *builtin_sub(lenv *e, lval *v);
lval *builtin_mul(lenv *e, lval *v);
lval *builtin_div(lenv *e, lval *v);
lval *builtin_head(lenv *e, lval *v);
lval *builtin_tail(lenv *e, lval *v);
lval *builtin_join(lenv *e, lval *v);
lval *builtin_list(lenv *e, lval *v);
lval *builtin_eval(lenv *e, lval *v);

// lval
lval *lval_pop(lval *sexpr, int idx);
lval *lval_take(lval *sexpr, int idx);
lval *lval_copy(lval *v);
lval *lval_join(lval *x, lval *y);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_eval(lenv *e, lval *v);

// read and construct
lval *lval_read(mpc_ast_t *t);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read_sym(mpc_ast_t *t);
lval *lval_add(lval *v, lval *x);

lval *lval_num(long x);
lval *lval_err(char *e);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_func(lbuiltin func);
void lval_del(lval *v);

// lenv
lenv *lenv_new(void);
void lenv_del(lenv *e);
void lenv_put(lenv *e, lval *k, lval *v);
lval *lenv_get(lenv *e, lval *k);
void lenv_add_builtin(lenv *e, char *name, lbuiltin func);
void lenv_add_builtins(lenv *e);

// some utils
void lval_sexpr_print(lval *v, char open, char close);
void lval_print(lval *v);
void lval_println(lval *v);

#endif
