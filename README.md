# brainrot

A small compiler written in C++20 for **brainrot**, a toy programming language.
Source files use the `.rot` extension. The compiler tokenizes the source, builds
an AST with a precedence-climbing parser, and emits x86-64 assembly which is
then assembled with NASM and linked with `ld` into a native Linux executable.

> This is a learning project for exploring how a compiler front-to-back works
> (lexer → parser → code generation → native binary).

## How it works

```
 .rot source ──▶ Tokenizer ──▶ Parser ──▶ Generator ──▶ out.asm ──▶ nasm ──▶ out.o ──▶ ld ──▶ out
```

| Stage       | File                | Responsibility                                              |
| ----------- | ------------------- | ----------------------------------------------------------- |
| Tokenizer   | `src/tokenizer.hpp` | Turns raw source text into a stream of `Token`s             |
| Parser      | `src/parser.hpp`    | Builds the AST; arithmetic via precedence climbing          |
| Generator   | `src/generator.hpp` | Walks the AST and emits x86-64 NASM assembly                |
| Arena       | `src/arena.hpp`     | Bump allocator used to allocate AST nodes                   |
| Driver      | `src/main.cpp`      | Reads the file, runs the pipeline, invokes `nasm` and `ld`  |

## Requirements

- A C++20 compiler (g++ / clang)
- CMake ≥ 3.20
- `nasm`
- `ld` (binutils)
- Linux on x86-64 (the generated code uses Linux syscalls)

On Debian/Ubuntu:

```sh
sudo apt install build-essential cmake nasm
```

## Building

```sh
cmake -B build
cmake --build build
```

This produces the compiler binary at `build/rot`.

## Usage

```sh
./build/rot <file.rot>
```

The input file **must** have a `.rot` extension. On success the compiler writes
three files to the current directory:

- `out.asm` — the generated assembly
- `out.o`   — the assembled object file
- `out`     — the linked executable

Run it and inspect the exit code:

```sh
./out
echo $?
```

## Example

`test.rot`:

```
let x = 1;
x = 10;
let y = 2;
let z = x + y;
exit(z);
```

Compile and run:

```sh
./build/rot test.rot
./out
echo $?   # prints 12  (10 + 2)
```

## Language reference

The language is intentionally minimal. Everything is a 64-bit integer.

### Statements

```
let x = 5;      // declare and initialize a variable
x = 10;         // reassign an existing variable
exit(x);        // exit the program with x as the exit status
```

### Expressions

- Integer literals: `42`
- Identifiers: `x`
- Arithmetic: `+`, `-`, `*`, `/` (with proper operator precedence)
- Parentheses: `(x + y) * 2`

```
let a = 2 + 3 * 4;    // 14, multiplication binds tighter
let b = (2 + 3) * 4;  // 20
```

### Grammar (informal)

```
prog   ::= stmt*
stmt   ::= "let" ident "=" expr ";"
         | ident "=" expr ";"
         | "exit" "(" expr ")" ";"
expr   ::= term (binop expr)*
term   ::= int | ident | "(" expr ")"
binop  ::= "+" | "-" | "*" | "/"
```

## Status / roadmap

Implemented:

- [x] Read source from file
- [x] Tokenizer
- [x] Precedence-climbing parser
- [x] Variable declaration and assignment
- [x] Arithmetic operations (`+ - * /`)
- [x] Code generation to x86-64 assembly
- [x] `exit()`

Planned / in progress:

- [ ] `if` / `elif` / `else` statements (tokens and AST exist; codegen is stubbed)
- [ ] Scopes / blocks (`{ ... }`)
- [ ] Loops
- [ ] Console output
- [ ] Comments

## Notes

- The generated program communicates its result through the process **exit
  code**, so values are effectively limited to the `0–255` range that Linux
  allows for exit statuses.
- The arena allocator (`src/arena.hpp`) reserves 5 MB up front for AST nodes
  and never frees individual nodes — the whole arena is released at once.
