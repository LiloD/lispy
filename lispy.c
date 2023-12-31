#include "mpc.h"
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

typedef struct traverse_stat {
  int num_of_nodes;
  int num_of_leaves;
  int num_of_branches;
  int max_children;
} traverse_stat;

traverse_stat bfs(mpc_ast_t *t);
long eval(mpc_ast_t *t);
long eval_op(long x, char *op, long y);

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

long eval(mpc_ast_t *t) {
  if (t == NULL) {
    return 0;
  }

  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  // now we know operator is ALWAYS stored in 2nd child
  char *op = t->children[1]->contents;

  // accumulate *op* start at 3rd child
  long accum = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    accum = eval_op(accum, op, eval(t->children[i]));
    i++;
  }

  return accum;
}

long eval_op(long x, char *op, long y) {
  if (strcmp(op, "+") == 0) {
    return x + y;
  }
  if (strcmp(op, "-") == 0) {
    return x - y;
  }
  if (strcmp(op, "*") == 0) {
    return x * y;
  }
  if (strcmp(op, "/") == 0) {
    return x / y;
  }
  return 0;
}

int main(int argc, char **argv) {
  // init mpc
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT, " \
             number : /-?[0-9]+/; \
             operator: '+' | '-' | '*' | '/'; \
             expr : <number> | '(' <operator> <expr>+ ')' ;\
             lispy : /^/ <operator> <expr>+ /$/ ; \
            ",
            Number, Operator, Expr, Lispy);

  puts("Lispy version 0.0.1");
  puts("Press Ctrl+c to exit");

  while (1) {
    char *input = readline("listp> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_print(r.output);

      mpc_ast_t *t = r.output;

      traverse_stat stat = bfs(t);
      printf("number of nodes in this ast is: %d\n", stat.num_of_nodes);
      printf("number of leaves in this ast is: %d\n", stat.num_of_leaves);
      printf("number of branches in this ast is: %d\n", stat.num_of_branches);
      printf("max of children: %d\n", stat.max_children);

      long result = eval(t);
      printf("result of eval: %ld\n", result);

      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_print(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}
