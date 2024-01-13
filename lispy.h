#ifndef lispy_h
#define lispy_h
#include "mpc.h"

// if condition not met, give an error and delete the args
#define LASSERT(args, cond, fmt, ...)                                          \
  if (!(cond)) {                                                               \
    lval *err = lval_err(fmt, ##__VA_ARGS__);                                  \
    lval_del(args);                                                            \
    return err;                                                                \
  }

#define LASSERT_COUNT(func_name, v, got, expect, prefix)                       \
  LASSERT(v, got->count == expect, "'%s' %s, Expect: %d, Got: %d", func_name,  \
          prefix, expect, got->count)

#define LASSERT_TYPE(func_name, v, got, expect, prefix)                        \
  LASSERT(v, got->type == expect, "'%s' %s, Expect: %s, Got: %s", func_name,   \
          prefix, get_type_name(expect), get_type_name(got->type))

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

  // basic type
  long num;
  char *sym;
  char *err;

  // functions, builtin and user defined
  lbuiltin builtin;
  lenv *env;
  lval *formals;
  lval *body;

  // s-expr and q-expr
  int count;
  struct lval **cell;
} lval;

typedef struct lenv {
  lenv *parent;
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
lval *builtin_def(lenv *e, lval *v);
lval *builtin_put(lenv *e, lval *v);
lval *builtin_var(lenv *e, lval *v, char *func);
lval *builtin_print_env(lenv *e, lval *v);
lval *builtin_exit(lenv *e, lval *v);
lval *builtin_lambda(lenv *e, lval *v);

// lval
lval *lval_pop(lval *sexpr, int idx);
lval *lval_take(lval *sexpr, int idx);
lval *lval_copy(lval *v);
lval *lval_join(lval *x, lval *y);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_eval(lenv *e, lval *v);
lval *lval_call(lenv *e, lval *v, lval *a);

// read and construct
lval *lval_read(mpc_ast_t *t);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read_sym(mpc_ast_t *t);
lval *lval_add(lval *v, lval *x);

lval *lval_num(long x);
lval *lval_err(char *fmt, ...);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_lambda(lval *foramls, lval *body);
lval *lval_builtin(lbuiltin func);
void lval_del(lval *v);

// lenv
lenv *lenv_new(void);
void lenv_del(lenv *e);
lenv *lenv_copy(lenv *e);
void lenv_put(lenv *e, lval *k, lval *v);
void lenv_def(lenv *e, lval *k, lval *v);
lval *lenv_get(lenv *e, lval *k);
void lenv_add_builtin(lenv *e, char *name, lbuiltin func);
void lenv_add_builtins(lenv *e);

// some utils
void lval_sexpr_print(lenv *e, lval *v, char open, char close);
void lval_print_func(lenv *e, lval *v);
void lval_print(lenv *e, lval *v);
void lval_println(lenv *e, lval *v);
char *get_type_name(int t);

#endif
