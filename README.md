This is a "tracing disassembler" for 6809 / 6309 binaries.

It works by starting at any of a given list of addresses in the
binary, and tracing instruction by instruction. As it traces it
updates a table in memory of whether each address is data, the
start of an instruction, additional instruction bytes, etc.

Every time it encounters a branch or subroutine call, or any other
instruction that predictably modifies the program counter, it
pushes the destination addresses onto its internal stack. Whenever
it runs out of code to trace on its current linear path, it pulls
an address off the internal stack and continues.

When the stack is empty, it outputs the disassembly based on its
internal table of what's at each address in the binary.

This has a few advantages over a conventional disassembler:
1) It generally won't try to disassemble data as code
2) It knows about the jump tables in common OS9 module types
   and can seed its stack with those entry points
3) It can output metadata to drive other disassemblers such
   as f9dasm

MIT license. Enjoy.
