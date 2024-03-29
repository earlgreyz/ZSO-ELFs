# Task 1: ELFs and aliens

## Extra materials

- [alienos.h](http://students.mimuw.edu.pl/ZSO/PUBLIC-SO/2017-2018/_build/html/_downloads/alienos.h)
- [prog](http://students.mimuw.edu.pl/ZSO/PUBLIC-SO/2017-2018/_build/html/_downloads/prog)

## Alien spaceship

Recently, an alien spaceship has crashed near the faculty. We managed
to extract data carriers containing programs for the very technically
advanced AlienOS operating system, but unfortunately no alien computer
system survived -- so we are not able to run their programs. However,
after many months of reverse engineering, we finally managed to find
out how the interfaces between the AlienOS kernel and user programs work.

## Task

Write a program that loads _AlienOS_ programs and emulates system calls,
allowing you to run _AlienOS_ programs under Linux. For testing, we attach
[an example program](http://students.mimuw.edu.pl/ZSO/PUBLIC-SO/2017-2018/_build/html/_downloads/prog)
extracted from the spacecraft (unfortunately, we have no idea what this
program does).  Your loader should of course work with any alien program
using the _AlienOS_ interface.

## Solution format

As the solution, you should deliver an archive containing:

- any number of __source files__ with the solution code
- a __Makefile__ that can build the solution, or a corresponding file from
  another sensible build system (eg cmake)
- a __readme__ file with a brief description of the solution and compilation
  instructions

The solution code should be written entirely in C (or C++ if you really want to)
and can use any sane libraries available in the Debian system. The solution
should compile to a program called `emu` that can be launched as follows:

``sh
  ./emu <alien program> [<parameters for alien program>]
``

After the alien program finishes running, the `emu` program should end with
the same exit code as the alien program.

The loader should check correctness of the input data to a reasonable degree.
In particular, it should be remembered that a crashed alien ship is not
a trusted source of code -- you must ensure that the code of the emulated
program can not do anything except using known system calls _(eg writing
/ reading arbitrary files)_. If an error is detected by the `emu` program,
the program should end with the __exit code 127__.

Please send the solutions to the address `marmarek@mimuw.edu.pl` with
a copy to `mwk@mimuw.edu.pl`.

## Scoring

You can get up to __10 points__ for the task.  The score is a sum of two parts:

- the result of testing (from 0 to 10 points)
- code assessment (from 0 to -10 points)

## Hints

Based on the documentation found on the spacecraft, our team of extraterrestrial
linguistics found that the example program is probably controlled by the arrow
keys, space bar, and enter. It takes one parameter whose sensible values are
in the range of 50-400. It requires a terminal size of at least 80×24.

The easiest way to emulate AlienOS system calls is to use `ptrace` with
the `PTRACE_SYSEMU` option.

To implement the `getrand` system call from the AlienOS system, we recommend
using the Linux system call `getrandom`, or a sensible PRNG initiated with such a call.

You can use the ncurses library to handle the terminal, or issue the appropriate
codes yourself (`man console_codes`).

The colors of the characters given in the documentation are just an approximation
(aliens see colors differently than humans) and you can use any reasonably
similar colors in the Linux terminal.

# Alien OS

## Binary format

It seems that the aliens are very advanced -- they use 64-bit x86 architecture
and ELF format.

Alien programs are statically linked ELF files of the type `ET_EXEC`
(it stands for ExtraTerrestial EXECutable, of course). They do
not use dynamic memory allocation and are always loaded at addresses in the range
`0x31337000 .. 0x80000000`. When starting the program, `rsp` should contain
the top address of some sensible stack, and the value of other registers is not
significant.

In addition to the standard `PT_LOAD` segments, these files may also contain
a segment of the special type `PT_PARAMS` (0x60031337). This segment, if it
exists, is used to pass parameters from the operating system. Parameters for
the program are always of type `int` and are passed in binary form (4 bytes,
little-endian). The size of the `PT_PARAMS` segment indicates how many
parameters a given program needs (`p_memsz / 4`). Before the program starts,
the operating system places parameter values in this segment. If the program
does not have such a segment, it means that it does not accept parameters.

## System calls

System calls are made using the `syscall` instruction.  The number of the
system call is passed in the register `rax`, the return value from the call
is also in the register `rax`, and the parameters are in the registers
`rdi`, `rsi`, `rdx` , `r10` (in that order).  The following known
system calls exist:

- 0: `void noreturn end(int status)`
- 1: `uint32_t getrand()`
- 2: `int getkey()`
- 3: `void print(int x, int y, uint16_t *chars, int n)`
- 4: `void setcursor(int x, int y)`

### `end`

Ends the program with the given exit code. The exit code should be in the range 0-63.

### `getrand`

Returns a random 32-bit number.

### `getkey`

Waits for pressing a key on the keyboard and returns his code. We know of
the following keycodes on alien keyboards:

- `0x0a`: enter
- `0x20`-`0x7e`: as in ASCII
- `0x80`: up arrow
- `0x81`: left arrow
- `0x82`: down arrow
- `0x83`: right arrow

There is no echo in the AlienOS system -- the keys pressed are not
automatically printed on the terminal.

### `print`

Print the given characters to the screen in the given (`x`, `y`) position.
Aliens use 80×24 text terminals. `x` means the column index, counted from
0 on the left. `y` means the row index, counted from 0 from the top of
the terminal. `chars` is a pointer to `n` characters to be printed in
the given line, starting from the given column and going to the right.
Each character is a 16-bit number with the following fields:

- bits 0-7: character code in ASCII (always in the range of `0x20 ... 0x7e` -- the aliens are not advanced enough yet to invent Unicode).
- bits 8-11: character color:

  - 0: black
  - 1: blue
  - 2: green
  - 3: turquoise
  - 4: red
  - 5: pink
  - 6: yellow
  - 7: light gray
  - 8: dark gray
  - 9: blue (bright)
  - 10: green (bright)
  - 11: turquoise (bright)
  - 12: red (bright)
  - 13: pink (bright)
  - 14: yellow (bright)
  - 15: white

- bits 12-15: don't seem to be used for anything

Calling `print` does not change the cursor position on the terminal.

### `setcursor`

Moves the cursor in the terminal to the given coordinates.
