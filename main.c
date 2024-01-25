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

mpc_parser_t *Symbol;
mpc_parser_t *Number;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Lispy;

int main(int argc, char **argv) {
  // init mpc
  Number = mpc_new("number");
  Symbol = mpc_new("symbol");
  Sexpr = mpc_new("sexpr");
  Qexpr = mpc_new("qexpr");
  Expr = mpc_new("expr");
  String = mpc_new("string");
  Lispy = mpc_new("lispy");
  Comment = mpc_new("comment");

  mpca_lang(MPCA_LANG_DEFAULT, " \
             number : /-?[0-9]+/; \
             symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/; \
             string: /\"(\\\\.|[^\"])*\"/; \
             sexpr: '(' <expr>* ')' ; \
             qexpr: '{' <expr>* '}' ; \
             comment: /#.*/ ; \
             expr : <comment> | <number> | <string> | <symbol> | <sexpr> | <qexpr>;\
             lispy : /^/ <expr>* /$/ ; \
            ",
            Comment, Number, Symbol, String, Sexpr, Qexpr, Expr, Lispy);

  // init the env and add default builtins
  lenv *e = lenv_new();
  lenv_add_builtins(e);

  if (argc == 1) {
    puts("Lispy version 0.0.1");
    puts("Press Ctrl+c to exit");

    while (1) {
      char *input = readline("lispy> ");

      add_history(input);

      mpc_result_t r;
      if (mpc_parse("<stdin>", input, Lispy, &r)) {
        mpc_ast_print(r.output);

        mpc_ast_t *t = r.output;

        lval *v = lval_read(t);
        lval_println(e, v);

        printf("value type:%d\n", v->type);
        printf("---------------------\n");
        v = lval_eval(e, v);
        lval_println(e, v);
        lval_del(v);
        mpc_ast_delete(r.output);
      } else {
        mpc_err_print(r.error);
        mpc_err_print(r.error);
      }

      free(input);
    }
  }

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));
      lval *x = builtin_load(e, args);

      if (x->type == LVAL_ERR) {
        lval_println(e, x);
      }
      lval_del(x);
    }
  }

  lenv_del(e);
  mpc_cleanup(7, Comment, Number, Symbol, String, Sexpr, Expr, Lispy);
  return 0;
}
