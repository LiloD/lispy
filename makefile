target := lispy

sources := main.c mpc.c lispy.c

links := -ledit

compiler := gcc

flags := -Wall -std=c99 -o $(target)

all: $(target)

$(target): $(sources)
		$(compiler) $^ $(flags) $(links) 
