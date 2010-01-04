
Introduction
------------

This page is a reference for Circa's syntax.

Comments
--------

Linewise comments are specified with two dashes: `--`. This will cause the rest of the line to be ignored.

    -- this is a comment

Currently there's no syntax for block comments, this is planned for the future.

Integers
--------

Examples:

    0, 1, -1, 1234

There is also hexadecimal syntax:

    0x00, 0xff0000ff

These create a value of type `int`.

Floating-point numbers
----------------------

Examples:

    0.0, .1, -.3, 123.456

These create a value of type `number`.

Exponent syntax is currently not supported (example: `1e9`), but planned for the future.

Strings
-------

Either single-quoted or double-quoted.

    'hello', "goodbye"

These create a value of type `string`.

Booleans
--------

Possibilities:

    true, false

These create a value of type `bool`

Colors
------

Has several different forms:

 * 3 hex digits (corresponding to red, green, blue)
 * 4 hex digits (as above, with 4th digit for alpha)
 * 6 hex digits (red, green, blue; two digits for each color)
 * 8 hex digits (as above, with last two digits for alpha).

Examples:

 * `#fff` (white)
 * `#f0f` (magenta)
 * `#00f8` (blue with 50% transparency)
 * `#ff4f00` (International orange)
 * `#00ff0088` (Green with 50% transparency)

Creates a value of type `Color`, which is a list of four numbers.

Function calls
--------------

A function call has the name of the function, then `(`, then a list of arguments, then `)`. Each argument is an expression, and they are separated by spaces or commas or semicolons.

Examples:

    add(1, 2, 3)
    add(1 2 3)
    rand()
    background(#000)
    print('hi')

Right-apply syntax
------------------

The `->` operator calls the function on the right with the value on the left. Example:
    'hi' -> print
is equivalent to:
    print('hi')

(This syntax only works for functions which take a single input; in a future version I'd like to add syntax for right-apply on functions with multiple inputs.)

Typecasting
-----------

A type name can be called like a function, this will attempt to cast the input to the specified type.
    a = number(1)
    print(a)     -- prints 1.0

Like function calls, you can also typecast with the right-apply operator (`->`):

    a = 1 -> number

Unary expressions
-----------------

There are three unary prefix operators:

<table>
<tr>
 <th>Operator</th><th>Function name</th><th>Descrption</th>
</tr>
<tr><td>-</td>      <td>neg</td><td>Unary negation</td></tr>
<tr><td>&</td>      <td>to_ref</td><td>Reference</td></tr>
<tr><td>@</td>      <td>(none)</td><td>Rebinding operator</td></tr>
</table>

A note of warning when using unary negation: if you have a list of items separated by spaces, sometimes whitespace will affect parsing. Examples:

 * `[3 - 2]` - creates a list of one element, equal to `[1]`
 * `[3 -2]` - creates a list of two elements, equal to `[3, -2]`
 * `[3-2]` - creates a list of one element, equal to `[1]`

Also, a side note about the `&` operator: references are not commonly used (as they are in C). Their primary usage is for writing code that does reflection/introspection.

The rebinding operator `@` is explained below.

Infix expressions
-----------------

We have the following infix operators:

<table>
<tr>
 <th>Operator</th><th>Function name</th><th>Descrption</th><th>Precedence</th>
</tr>
<tr>
 <tr><td>.</td>      <td>(multiple)</td><td>Named field access or member function call</td><td>9</td></tr>
 <tr><td>::</td>      <td>annotate_type</td><td>Specifies a value's type</td><td>8</td></tr>
 <tr><td>..</td>      <td>range</td><td>Range of integers</td><td>8</td></tr>
 <tr><td>*</td>      <td>mult</td><td>Multiplication (overloaded)</td><td>7</td></tr>
 <tr><td>/</td>      <td>div</td><td>Division, floating-point</td><td>7</td></tr>
 <tr><td>//</td>      <td>div_i</td><td>Division, integral</td><td>7</td></tr>
 <tr><td>%</td>      <td>remainder</td><td>Modulo, truncated division</td><td>7</td></tr>
 <tr><td>+</td>      <td>add</td><td>Addition (overloaded)</td><td>6</td></tr>
 <tr><td>-</td>      <td>sub</td><td>Subtraction (overloaded)</td><td>6</td></tr>
 <tr><td>-></td>      <td>(none)</td><td>Right-apply</td><td>5</td></tr>
 <tr><td><</td>      <td>less_than</td><td>Less-than comparison</td><td>4</td></tr>
 <tr><td><=</td>      <td>less_than_eq</td><td>Less-than-or-equals comparison</td><td>4</td></tr>
 <tr><td>></td>      <td>greater_than</td><td>Greater-than comparison</td><td>4</td></tr>
 <tr><td>>=</td>      <td>greater_than_eq</td><td>Greater-than-or-equals comparison</td><td>4</td></tr>
 <tr><td>==</td>      <td>equals</td><td>Equality check</td><td>4</td></tr>
 <tr><td>!=</td>      <td>equals</td><td>Inequality check</td><td>4</td></tr>
 <tr><td>and</td>      <td>and</td><td>Logical and</td><td>3</td></tr>
 <tr><td>or</td>      <td>and</td><td>Logical or</td><td>2</td></tr>
 <tr><td>+=</td>      <td>add</td><td>Rebinding add</td><td>2</td></tr>
 <tr><td>-=</td>      <td>sub</td><td>Rebinding sub</td><td>2</td></tr>
 <tr><td>*=</td>      <td>mult</td><td>Rebinding mult</td><td>2</td></tr>
 <tr><td>/=</td>      <td>div</td><td>Rebinding div</td><td>2</td></tr>
</table>

The `/` operator always does floating-point division, it isn't overloaded as in C.

The 'rebinding' operators will perform the advertised operation, and then bind the result to the name of the identifier on the left. Example:
    
    a += 1

is the same as:
   
    a = a + 1

Name bindings
-------------

A name binding statement is of the form:

    <identifier> = <expression>

Some examples:

    one = 1
    four = 2 + 2
    output = concat('There are ' number_of_things ' things)

Rebind operator
------------------

The `@` operator can be put in front of an identifier. This causes the result of the overall expression to be bound to the specified name. Example:

    a = 1
    add(@a, 2)

has the same effect as:

    a = add(a, 2)

Lists
-----

Lists are specified with the `[` and `]` symbols. The elements of a list can be separated by spaces or commas or semicolons (just like function call arguments).

    []
    [1 2 3]
    ['apple', 'banana', 'cucumber']

Indexed access
------------

The `[]` symbols are used to access an element in a list by index. Indices are zero-based.

    example_list = [1 2 3]
    example_list[0]     -- equals 1
    example_list[1]     -- equals 2
    example_list[2]     -- equals 3

Note that there must not be any whitespace between the expression on the left and the `[` symbol, otherwise the right side will be parsed as a list.

    print(example_list[0])   -- prints '1'
    print(example_list [0])  -- prints '[1 2 3][0]'

If statement
------------

An if-statement is specified by the `if` keyword, followed by a conditional expression, followed by a list of statements, followed by the `end` keyword. The list of statements are executed if the conditional expression is true. Examples:

    if mouse_pressed()
        print('The mouse is pressed')
    end

    if a > b; print('a is higher'); end

Statements can be separated by line breaks, commas, or semicolons, but they are optional. The above line would work fine if it looked like this:

    if a > b print('a is higher') end

An if-statement can also have an `else` section, which is evaluated if the conditional expression is false. Examples:

    if a > b
        highest = a
    else
        highest = b
    end

There is also the `elif` keyword, which is followed by another conditional expression. There can be many of these.

    if a > b
        print('a is higher')
    elif b > a
        print('b is higher')
    else
        print('a equals b')
    end

For loop
-------------

A for-loop is specified by the `for` keyword. The first line has this syntax:

    for <iterator name> in <list expression>

which is followed by a list of expressions, and then the `end` keyword. Example:

    for i in [0, 1, 2, 3]
        print('i is ' i)
    end

When executing a for-loop, it goes through each element in the list expression, binds that value to the iterator name, and executes the inner statements.

This piece of code:

    for i in [0 1 2 3 4]
        print(i ' squared is ' i*i)
    end

has the following output:

    0 squared is 0
    1 squared is 1
    2 squared is 4
    3 squared is 9
    4 squared is 16

When iterating across a range of numbers, it's convenient to use the range operator (`..`). The range operator takes two integers and outputs a list of each integer within that range. The left input is included and the right input is excluded. Example:

    a = 0..4
    -- a is now equal to: [0 1 2 3]

The above for-loop can be rewritten as:

    for i in 0..5
        print(i ' squared is ' i*i)
    end

For loop with rebinding
----------------------------

A for-loop can also *rebind* the list expression using the `@` operator, which means that the inner code is allowed to modify each item, and the list name is rebound to the modified list. Example:

    numbers = [0 1 2 3 4]
    for i in @numbers
        i = i*i
    end
    print(numbers)

will print the result: `[0, 1, 4, 9, 16]`.

A rebinding for-statement can use the special keyword `discard`, which means that it will remove the current element from the output. The following for-loop will discard any odd numbers from the `numbers` list:

    numbers = 0..10
    -- numbers is now equal to [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for i in @numbers 
        if i % 2 == 1
            discard
        end
    end
    -- numbers is now equal to [0, 2, 4, 6, 8]

Function declarations
---------------------

A function can be declared with the `def` keyword. Example:

    def <function-name>(<list of arguments>) -> <optional-return-type>
        (statements)
    end

An example:

    def pythag(number a, number b) -> number
        return sqrt(a*a + b*b)
    end

The `->` in the header indicates that the function has a return type. This is optional:

    def print_warning(string message)
        print('warning: ' message)
    end

Once a function is declared, it can then be called like a normal function.

    pythag(3, 4)           -- returns 5
    print_warning('hi')    -- prints 'warning: hi'

Compound types
--------------

Compound types can be declared using the `type` keyword, and providing a list of typed fields inside brackets. An example:

    type Complex {
        number real
        number imag
    }

Fields can be seperated by commas, semicolons, or newlines:

    type Triangle { Point a, Point b, Point c }

A compound value can then be created using constructor syntax, or by taking an existing compound value and casting it.

    complex_value = Complex()
    complex_value = [1.0 0.0] -> Complex

Side note, we don't currently support constructors with arguments (such as `Complex(1.0 0.0)`), but this is planned for a future version.

Fields of a compound value can be accessed or assigned with the dot operator. Example:

    real_part = complex_value.real
    complex_value.imag = 1.0
    

Namespaces
----------

Code can be organized into a namespace. The syntax is:

    namespace <name>
        (statements)
    end

Example:

    namespace web
        def get_page(string url) :: string
            ...
        end
    end

To access something inside a namespace, use a colon-seperated identifier:

    contents = web:get_page(url)