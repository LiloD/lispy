#include "lispy.h"
#include <stdlib.h>
#include <string.h>

lenv *lenv_new(void) {
  lenv *e = malloc(sizeof(lenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv *e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }

  free(e->syms);
  free(e->vals);
  free(e);
}

void lenv_put(lenv *e, lval *k, lval *v) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(k->sym, e->syms[i]) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  e->count++;
  e->syms = realloc(e->syms, sizeof(char *) * e->count);
  e->vals = realloc(e->vals, sizeof(lval *) * e->count);

  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

lval *lenv_get(lenv *e, lval *k) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(k->sym, e->syms[i]) == 0) {
      lval *ret = lval_copy(e->vals[i]);
      return ret;
    }
  }

  return lval_err("Unbound symbol: %s", k->sym);
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func) {
  lval *k = lval_sym(name);
  lval *v = lval_func(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}
