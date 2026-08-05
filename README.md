This is a "tracing disassembler" for 6809 / 6309 binaries.

It works by starting at any of a given list of addresses in the binary, and tracing instruction by instruction. As it traces it updates a table in memory of whether each address is data, the start of an instruction, additional instruction bytes, etc.

Every time it encounters a branch or subroutine call, or any other instruction that predictably modifies the program counter, it pushes the destination addresses onto its internal stack. Whenever it runs out of code to trace on its current linear path, it pulls an address off the internal stack and continues.

When the stack is empty, it outputs the disassembly based on its internal table of what's at each address in the binary.

This has a few advantages over a conventional disassembler:
* It generally won't try to disassemble data as code
* It knows about the jump tables in common OS9 module types and can seed its stack with those entry points
* It's OS9-aware but can also disassemble any 6809/6309 binary
* The output format is designed to be "diffable" -- for example when comparing disassembly of a baseline and customized module
* It can output metadata to drive other disassemblers such as [f9dasm](https://www.hermannseib.com/english/opensource.htm)

```
Usage:

diffdasm <options> <module>
Disassemble 6809 OS9 module to a diffable format.

Options:
--base xxxx Specifies a hex base address (defaults to zero)
--exec xxxx Specifies a hex execution address. Can use multiple times.
--source Output in assembler source format rather than diff format.
--f9info Output in f9dasm info file format rather than diff format.
--ioflag Call out potential references to (Color Computer) I/O.
--debug  Output debugging information.
```

License:

MIT license. Enjoy.

Chris Burke 8-5-26
