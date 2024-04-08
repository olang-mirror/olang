% O programming language specification

Abstract
--------

This document specifies the semantics and behavior of the O Programming
Language for compiler designers be informed how the language is designed.

This specification is a DRAFT and will be the discussions drive over olang-dev
mailing list.

Language Syntax
---------------

This is the O Programming Language EBNF grammar specification[^1]

[^1]: EBNF variant https://github.com/Engelberg/instaparse/tree/v1.4.12 and live
      test can be accessed here https://mdkrajnak.github.io/ebnftest/

NOTE: This grammar spec is a DRAFT and it covers only a small portion of the
language.

```
(* Entry Point *)
<translation-unit>     ::= (<ows> <external-declaration> <ows> (<end-of-statement> | <end-of-file>))*

<external-declaration> ::= <function-definition> | <variable-definition>

(* Variables *)
<variable-definition>  ::= <variable-qualifier> <ws> <variable-name> <ows> ':' <ows> <type> (<ows> <assign-operator> <ows> <expression>)?
<variable-qualifier>   ::= 'var'
                         | 'const'
<variable-name>        ::= <identifier>

(* Functions *)
<function-definition> ::= 'fn' <ws> <function-name> <ows> <function-parameters> <ows> ':' <ows> <return-type> <ows> <function-body>
<function-name>       ::= <identifier>
<function-parameters> ::= '(' <ows> ')'
<return-type>         ::= <type>
<function-body>       ::= <block>

(* Statements *)
<block>               ::= '{' <ows> <statement> <ows> (<end-of-statement> <ows> <statement> <ows>)* <end-of-statement>? <ows> '}'
<end-of-statement>    ::= ';' | <line-break>
<statement>           ::= <return-statement> | <variable-definition> | <assign-expression>
<return-statement>    ::= 'return' <ws> <expression>

(* Expressions *)
<expression>          ::= <integer> | <identifier>
<assign-expression>   ::= <variable-name> <ows> <assign-operator> <ows> <expression>
<assign-operator>     ::= '='
                        | '*='
                        | '/='
                        | '%='
                        | '+='
                        | '-='
                        | '<<='
                        | '>>='
                        | '&='
                        | '^='
                        | '|='

(* Identifiers *)
<type>                ::= 'u32'
<identifier>          ::= (<alpha> | '_') (<alpha> | <digit> | '_')*

(* Literals *)
<integer>             ::= <integer-base10> | <integer-base16>
<integer-base10>      ::= #'[1-9]' (<digit> | '_')* | '0'
<integer-base16>      ::= #'0[Xx]' <hex-digit> (<hex-digit> | '_')*

(* Utilities *)
<ws>                  ::= <white-space>+
<ows>                 ::= <white-space>*
<white-space>         ::= <linear-space> | <line-break>
<line-break>          ::= #'[\n\v\f\r]' | '\r\n'
<linear-space>        ::= #'[ \t]'
<alpha>               ::= #'[a-zA-Z]'
<digit>               ::= #'[0-9]'
<hex-digit>           ::= <digit> | #'[a-fA-F]'
<end-of-file>         ::= #'$'
```
