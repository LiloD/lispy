#include "lispy.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>
static char buf[2048];

char *readline(char *prompt) {
  fputs("listp> ", stdout);
  fgets(input, 2048, stdin);
  char *copy = malloc(strlen(buf) + 1);
  strcpy(copy, buf);
  copy[strlen(cpy) - 1] = '\0';
  return cpy;
}

void add_history(char *unused) {}

#else
#include <editline/readline.h>
#endif
int main(int argc, char **argv) {
  // init mpc
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT, " \
             number : /-?[0-9]+/; \
             symbol: '+' | '-' | '*' | '/'; \
             sexpr: '(' <expr>* ')' ; \
             expr : <number> | <symbol> | <sexpr> ;\
             lispy : /^/ <expr>* /$/ ; \
            ",
            Number, Symbol, Sexpr, Expr, Lispy);

  puts("Lispy version 0.0.1");
  puts("Press Ctrl+c to exit");

  while (1) {
    char *input = readline("lispy> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_print(r.output);

      mpc_ast_t *t = r.output;

      // traverse_stat stat = bfs(t);
      // printf("number of nodes in this ast is: %d\n", stat.num_of_nodes);
      // printf("number of leaves in this ast is: %d\n", stat.num_of_leaves);
      // printf("number of branches in this ast is: %d\n",
      // stat.num_of_branches); printf("max of children: %d\n",
      // stat.max_children);

      // lval result = eval(t);
      // lval_println(result);

      lval *v = lval_read(t);
      lval_println(v);
      printf("---------------------\n");
      v = lval_eval(v);
      lval_println(v);
      lval_del(v);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_print(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Symbol, Sexpr, Expr, Lispy);
  return 0;
}
