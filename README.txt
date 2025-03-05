# Parser
Develop a parser using the Recursive Descent Parsing technique to determine whether a program input through a file follows the LL(1) grammar below using a single lookahead symbol, and output the results of the input program using the parsed results.

<program> → <statements>
<statements> → <statement> | <statement><semi_colon><statements>
<statement> → <ident><assignment_op><expression>
<expression> → <term><term_tail>
<term_tail> → <add_op><term><term_tail> | ε
<term> → <factor> <factor_tail>
<factor_tail> → <mult_op><factor><factor_tail> | ε
<factor> → <left_paren><expression><right_paren> | <ident> | <const>
<const> → any decimal numbers
<ident> → any names conforming to C identifier rules
<assignment_op> → :=
<semi_colon> → ;
<add_operator> → + | -
<mult_operator> → * | /
<left_paren> → (
<right_paren> → )


(a) Output should be :
"Lines read from the program"
"ID: {Number}; CONST: {Number}; OP: {Number};"
"Parsing result (OK), (WARNING), (ERROR)"
"Result ==> {variable 1}: {final value}; {variable 2}: {final value}; {variable 3}: {final value};"

if the option is "-v"
(b) Output should be : 
"token1"
"token2"
"token3"
"..."

if warning or error : 
If an error is found based on the grammar given above, the error is repaired as much as possible and then parsing continues. 
ex) x = a + + b, there is one more “+” operator, so the “+” symbol is removed, an appropriate ‘warning message’ is output, and parsing continues. 
If error recovery is not possible, an ‘Error message’ is output and parsing continues, but in this case, the value of the identifier (<IDENT>) is determined to be ‘Unknown’.
