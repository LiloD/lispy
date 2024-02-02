## Q-List Operation
list - convert a S-expression to Q-expression

```
(list +) => {+}

(list 1 2 3 4) => {1 2 3 4}
```

join - join multiple Q-expressions to one Q-expression 
```
(join {1} {2} {3}) => {1 2 3}

(join {+} {1 2}) => {+ 1 2}
```

eval - convert a Q-expression to S-expression and evaluate the S-expression and return the result value
```
(eval (join (list +) {1 2})) => (eval (join {+} {1 2}))=> (eval {+ 1 2}) => (+ 1 2) => 3
```

## Function
declare a anonymous function 
```
# declare a function with no argument
(\ {} { print "hello world" })

# declare a function with 2 argument
(\ {x y} {+ x y}) 

# create a function that add two numbers together and assgin to `add2`
(def {add-together} (\ {x y} {+ x y}))

# call this function
(add-together 1 2) => 3
```

it's also possible to create a function with variable arguments just like + operator 
by using `&` inside argument list
```
# define a function with variable arguments
(def {foo} (
    \ {a & args} {
        print a args
    }
))

# call the function
(foo 1 2 3 4)

# as you can see, print a result in 1, print args result in a list contains all other arguments
1 {2 3 4}
```

when calling a function, you can assign partial arguments, which you can get another function that is partially evaluated
```
# create a new function `add10` from add-together created above
(def {add10} (add-together 10))

# now you can call add10 which you only need to pass one more argument
(add10 100) => 110
```


declare a named function (define a anonymous function and assign it to a variable)
```
# function name and arguments are defined in one Q-expression
(func {add-together x y} {
    + x y
})

# call this function
(add-together 1 2) => 3
```

## some interesting function
Because of the reasons
1. variable can be data or function
2. variable can be contained inside a list
3. list can be manipulate by list operation
4. list (q-expression) can be evaluated by `eval`
so a lot of interesting things can be done


`func` keyword is not a bulitin but defined in lisp
```
(def {func} (
    / {args body} {
       def (head args) (\ (tail args) body) 
    }
))
```

`unpack` unpack a list of values(a list is considered one argument) into multiple arguments 
```
(func {unpack f xs} {
    eval (join (list f) xs)
})

(unpack + {1 2 3 4}) => 10
```


`pack` pack (the opposite of unpack) pack multiple arguments into one list(one argument)
```
(func {pack f & args} {
   f args 
})

(pack head 1 2 3) => {1} 
```


## implement IF
```
(if number {} {})
number -> 0 | non-0

(if 1 {
    print A
} {
    print B
})
```

