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

(* Translation Unit *)
<external-declaration> ::= <common-statement> | <function-definition>

(* Variables *)
<variable-definition>   ::= 'var' <ws> <variable-name> <ows> ':' <ows> <type> <ows> <variable-initializer>?
<constant-definition>   ::= 'const' <ws> <variable-name> <ows> ':' <ows> <type> <ows> <variable-initializer>
<variable-name>         ::= <identifier>
<variable-initializer>  ::= '=' <ows> <expression>

(* Functions *)
<function-definition> ::= 'fn' <ws> <function-name> <ows> <function-parameters> <ows> ':' <ows> <return-type> <ows> <function-body>
<function-name>       ::= <identifier>
<function-parameters> ::= '(' <ows> ')'
<return-type>         ::= <type>
<function-body>       ::= <block>
<block>               ::= '{' <ows> <statement> <ows> (<end-of-statement> <ows> <statement> <ows>)* <end-of-statement>? <ows> '}'
<statement>           ::= <common-statement> | <return-statement>
<return-statement>    ::= 'return' <ws> <expression>

(* Statements *)
<end-of-statement>    ::= ';' | <line-break>
<common-statement>    ::= <variable-definition> | <assignment-expression>

(* Expressions *)
<expression>            ::= <integer-literal> | <variable-name>
<assignment-expression> ::= <variable-name> <ows> <assignment-operator> <ows> <expression>
<assignment-operator> ::= '='
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
<integer-literal>     ::= <integer-base10> | <integer-base16>
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
