# Asembler and linker

Implementation of a compiler toolchain for abstract computer system realized as a school assignment for System Software course.
Description of a computer system can be found in SS_projekat.pdf

## Asembler branch

Two-pass asembler which generates text file similar to the objdump output, and its binary representation for linker input.

`make`<br>
`./asembler -o <output_filename> <input_filename>`<br>
e.g. `./asembler -o main.o main.s`

## Linker branch

`make`<br>
`./linker [options] <input_filename>`<br>
options: <br>
`-o <input_filename>` <br>
`-place=<section_name>@<address>` <br>
`-hex` <br>
`-linkable` <br>

e.g. `linker -hex -place=iv_table@0x0000 -place=text@0x4000 -o mem_content.hex ulaz1.o ulaz2.o`

Examples can be found in tests folder.

