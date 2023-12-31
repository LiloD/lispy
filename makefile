target := lispy

sources := mpc.c prompt.c

links := -ledit

compiler := gcc

flags := -Wall -std=c99 -o $(target)

all: $(target)

$(target): $(sources)
		$(compiler) $^ $(flags) $(links) 
