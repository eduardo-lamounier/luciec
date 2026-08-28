# Lucie's Compiler

`luciec` is a compiler written in C for the Lucie programming language.

## Observations

This project is still in development, so it doesn't have all the intended functionalities. In the moment, `luciec` is only capable of parsing a simple expression, such as:

$2 + 3 / 4$, that is parsed into $+( 2, /( 3, 4 ) )$

$(2 + 3) / 4 == 0$, that is parsed into $==( /( +( 2, 3 ), 4 ), 0 )$

## Collaborating

For now I'm only accepting feedback by email of possible improvements (and please do!), but because I still want to write the code, I won't accept any PR at the moment.

My contact email is `eduardolamounierm@gmail.com`.
