target := lispy

sources := main.c mpc.c lval.c lenv.c builtin.c utils.c

links := -ledit

compiler := gcc

flags := -Wall -std=c11 -g -O0 -o $(target)

all: $(target)

$(target): $(sources)
		$(compiler) $^ $(flags) $(links) 
