
This is an introduction to doing research with the LLVM compiler infrastructure[1]. It should be enough to go from mostly uninterested in compilers to excited to use LLVM to do great work.

# LLVM’s architecture

LLVM IR is a low-level intermediate representation used by the LLVM compiler framework. You can think of LLVM IR as a platform-independent assembly language with an infinite number of function local registers.

When developing compilers there are huge benefits with compiling your source language to an intermediate representation (IR)1 instead of compiling directly to a target architecture (e.g. x86). As many optimization techniques are general (e.g. dead code elimination, constant propagation), these optimization passes may be performed directly on the IR level and thus shared between all targets2.

Compilers are therefore often split into three components, the front-end, middle-end and back-end; each with a specific task that takes IR as input and/or produces IR as output.

Here’s a picture that shows the major components of LLVM’s architecture (and, really, the architecture of any modern compiler):

![compiler-arch](res/compiler-arch.svg)

There are:

1. The front end, which takes your source code and turns it into an intermediate representation or IR. This translation simplifies the job of the rest of the compiler, which doesn’t want to deal with the full complexity of C++ source code. The intrepid you, probably do not need to hack this part; you can use Clang unmodified.

2. The passes, which transform IR to IR. In ordinary circumstances, passes usually optimize the code: that is, they produce an IR program as output that does the same thing as the IR they took as input, except that it’s faster. This is where you want to hack. Your research tool can work by looking at and changing IR as it flows through the compilation process.

3. The back end, which generates actual machine code. You almost certainly don’t need to touch this part.

Although this architecture describes most compilers these days, one novelty about LLVM is worth noting here: **programs use the same IR throughout the process**. In other compilers, each pass might produce code in a unique form. LLVM opts for the opposite approach, which is great for us as hackers: we don’t have to worry much about when in the process our code runs, as long as it’s somewhere between the front end and back end.

The llvm framework could used for different front-end and back-end.[8]
![compiler-arch](res/llvm_compiler_pipeline.jpg)

# Getting Start
## 1. Get LLVM
- git repository from https://github.com/llvm/llvm-project/
```
    git clone git@github.com:llvm/llvm-project.git
```
- build llvm  
Follow the [Building LLVM with CMake](http://llvm.org/docs/CMake.html) and [Getting Started: Building and Running Clang](http://clang.llvm.org/get_started.html)

    https://github.com/llvm/llvm-project/blob/master/llvm/docs/CMake.rst#id25

    

## 2. RTFM

- [Clang : a C language family frontend for LLVM : http://clang.llvm.org/](http://clang.llvm.org/)

- [LLVM Language Reference Manual : http://llvm.org/docs/LangRef.html](http://llvm.org/docs/LangRef.html)  
The language reference manual is handy if you ever get confused by syntax in an LLVM IR dump.

- [LLVM Programmer’s Manual: http://llvm.org/docs/ProgrammersManual.html](http://llvm.org/docs/ProgrammersManual.html)  
The programmer’s manual describes the toolchest of data structures peculiar to LLVM, including efficient strings, STL alternatives for maps and vectors, etc. It also outlines the fast type introspection tools (isa, cast, and dyn_cast) that you’ll run into everywhere.
- Writing an LLVM Pass
Read the [Writing an LLVM Pass](http://llvm.org/docs/WritingAnLLVMPass.html) tutorial whenever you have questions about what your pass can do. Because you’re a researcher and not a day-to-day compiler hacker, this article disagrees with that tutorial on some details. (Most urgently, ignore the Makefile-based build system instructions and skip straight to the [CMake-based “out-of-source” instructions](http://llvm.org/docs/CMake.html#cmake-out-of-source-pass).) But it’s nonetheless the canonical source for answers about passes in general.


# Understanding LLVM IR
To work with programs in LLVM, you need to know a little about how the IR is organized.

## Containers
![Modules](res/llvm-containers.svg)     
 

Here’s on overview of the most important components in an LLVM program : (Modules contain Functions, which contain BasicBlocks, which contain Instructions. Everything but Module descends from Value)  
- A Module represents a source file (roughly) or a translation unit (pedantically). Everything else is contained in a Module.
- Modules house Functions, which are exactly what they sound like: named chunks of executable code. (In C++, both functions and methods correspond to LLVM Functions.)
- Aside from declaring its name and arguments, a Function is mainly a container of BasicBlocks. The basic block is a familiar concept from compilers, but for our purposes, it’s just a contiguous chunk of instructions.
- An Instruction, in turn, is a single code operation. The level of abstraction is roughly the same as in RISC-like machine code: an instruction might be an integer addition, a floating-point divide, or a store to memory, for example.  

Most things in LLVM—including Function, BasicBlock, and Instruction—are C++ classes that inherit from an omnivorous base class called Value. A Value is any data that can be used in a computation—a number, for example, or the address of some code. Global variables and constants (a.k.a. literals or immediates, like 5) are also Values.

## Instruction
if you ever want to see the LLVM IR for your program, you can instruct Clang to do that:
```
$ clang -emit-llvm -S -o - something.c
```
Here’s an example of an Instruction in the human-readable text form of LLVM IR:
```
%5 = add i32 %4, 2
```
This instruction adds two 32-bit integer values (indicated by the type i32). It adds the number in register 4 (written %4) and the literal number 2 (written 2) and places its result in register 5. This is what I mean when I say LLVM IR looks like idealized RISC machine code: we even use the same terminology, like register, but there are infinitely many registers.  
That same instruction is represented inside the compiler as an instance of the Instruction C++ class. The object has an opcode indicating that it’s an addition, a type, and a list of operands that are pointers to other Value objects. In our case, it points to a Constant object representing the number 2 and another Instruction corresponding to the register %4. (Since LLVM IR is in static single assignment form, registers and Instructions are actually one and the same. Register numbers are an artifact of the text representation.)

## Write a Pass
Writing an LLVM Pass : http://llvm.org/docs/WritingAnLLVMPass.html
llvm-pass-skeleton : https://github.com/sampsyo/llvm-pass-skeleton

Now Make the Pass Do Something Mildly Interesting
The real magic comes in when you look for patterns in the program and, optionally, change the code when you find them. Here’s a really simple example: let’s say we want to replace the first binary operator (+, -, etc.) in every function with a multiply. Sounds useful, right?

Here’s the code to do that. This version, along with an example program to try it on, is available in the mutate branch of the llvm-pass-skeleton git repository:
```
for (auto& B : F) {
  for (auto& I : B) {
    if (auto* op = dyn_cast<BinaryOperator>(&I)) {
      // Insert at the point where the instruction `op` appears.
      IRBuilder<> builder(op);

      // Make a multiply with the same operands as `op`.
      Value* lhs = op->getOperand(0);
      Value* rhs = op->getOperand(1);
      Value* mul = builder.CreateMul(lhs, rhs);

      // Everywhere the old instruction was used as an operand, use our
      // new multiply instruction instead.
      for (auto& U : op->uses()) {
        User* user = U.getUser();  // A User is anything with operands.
        user->setOperand(U.getOperandNo(), mul);
      }

      // We modified the code.
      return true;
    }
  }
}
```
Details:
- That dyn_cast<T>(p) construct is an LLVM-specific introspection utility. It uses some conventions from the LLVM codebase to made dynamic type tests efficient, because compilers have to use them all the time. This particular construct returns a null pointer if I is not a BinaryOperator, so it’s perfect for special-casing like this.
- The IRBuilder is for constructing code. It has a million methods for creating any kind of instruction you could possibly want.
- To stitch our new instruction into the code, we have to find all the places it’s used and swap in our new instruction as an argument. Recall that an Instruction is a Value: here, the multiply Instruction is used as an operand in another Instruction, meaning that the product will be fed in as an argument.   

We should probably also remove the old instruction, but I left bit that off for brevity.
Now if we compile a program like this (example.c in the repository):
```
#include <stdio.h>
int main(int argc, const char** argv) {
    int num;
    scanf("%i", &num);
    printf("%i\n", num + 2);
    return 0;
}
```
Compiling it with an ordinary compiler does what the code says, but our plugin makes it double the number instead of adding 2:
```
$ cc example.c
$ ./a.out
10
12
$ clang -Xclang -load -Xclang build/skeleton/libSkeletonPass.so example.c
$ ./a.out
10
20
```
Like magic!

## Compiling Link With a Runtime Library

When you need to instrument code to do something nontrivial, it can be painful to use IRBuilder to generate the LLVM instructions to do it. Instead, you probably want to write your run-time behavior in C and link it with the program you’re compiling. This section will show you how to write a runtime library that logs the results of binary operators instead of silently changing them.

Here’s the LLVM pass code, which is in the rtlib branch of the llvm-pass-skeleton repository:
```
// Get the function to call from our runtime library.
LLVMContext& Ctx = F.getContext();
Constant* logFunc = F.getParent()->getOrInsertFunction(
  "logop", Type::getVoidTy(Ctx), Type::getInt32Ty(Ctx), NULL
);

for (auto& B : F) {
  for (auto& I : B) {
    if (auto* op = dyn_cast<BinaryOperator>(&I)) {
      // Insert *after* `op`.
      IRBuilder<> builder(op);
      builder.SetInsertPoint(&B, ++builder.GetInsertPoint());

      // Insert a call to our function.
      Value* args[] = {op};
      builder.CreateCall(logFunc, args);

      return true;
    }
  }
}
```
The tools you need are Module::getOrInsertFunction and IRBuilder::CreateCall. The former adds a declaration for your runtime function logop, which is analogous to declaring void logop(int i); in the program’s C source without a function body. The instrumentation code pairs with a run-time library (rtlib.c in the repository) that defines that logop function:
```
#include <stdio.h>
void logop(int i) {
  printf("computed: %i\n", i);
}
```
To run an instrumented program, link it with your runtime library:
```
$ cc -c rtlib.c
$ clang -Xclang -load -Xclang build/skeleton/libSkeletonPass.so -c example.c
$ cc example.o rtlib.o
$ ./a.out
12
computed: 14
14
```
If you like, it’s also possible to stitch together the program and runtime library before compiling to machine code. The [llvm-link utility](http://llvm.org/docs/CommandGuide/llvm-link.html), which you can think of as the rough IR-level equivalent of [ld](https://sourceware.org/binutils/docs/ld/), can help with that.

## Annotations
Most projects eventually need to interact with the programmer. You’ll eventually wish for annotations: some way to convey extra information from the program to your LLVM pass. There are several ways to build up annotation systems:

- The practical and hacky way is to use magic functions. Declare some empty functions with special, probably-unique names in a header file. Include that file in your source and call those do-nothing functions. Then, in your pass, look for CallInst instructions that invoke your functions and use them to trigger your magic. For example, you might use calls like __enable_instrumentation() and __disable_instrumentation() to let the program confine your code-munging to specific regions.
- If you need to let programmers add markers to function or variable declarations, Clang’s __attribute__((annotate("foo"))) syntax will emit [metadata](http://llvm.org/docs/LangRef.html#metadata) with an arbitrary string that you can process in your pass. Brandon Holt again has [some background on this technique](http://bholt.org/posts/llvm-quick-tricks.html). If you need to mark expressions instead of declarations, the undocumented and sadly limited[__builtin_annotation(e, "foo") intrinsic](https://github.com/llvm-mirror/clang/blob/master/test/Sema/annotate.c) might work.
- You can jump in full dingle and modify Clang itself to interpret your new syntax. I don’t recommend this.
- If you need to annotate types—and I believe people often do, even if they don’t realize it—I’m developing a system called [Quala](https://github.com/sampsyo/quala). It patches Clang to support custom type qualifiers and pluggable type systems, à la [JSR-308](https://checkerframework.org/jsr308/) for Java. Let me know if you’re interested in collaborating on this project!
I hope to expand on some of these techniques in future posts.

## FurtherMore
LLVM is enormous. Here are a few more topics I didn’t cover here:

- Using the vast array of classic compiler analyses available in LLVM’s junk drawer.
- Generating any special machine instructions, as architects often want to do, by hacking the back end.
Exploiting [debug info](http://llvm.org/docs/SourceLevelDebugging.html)[6], so you can connect back to the source line and column corresponding to a point in the IR.
- Writing [frontend plugins for Clang](http://clang.llvm.org/docs/ClangPlugins.html).
- I hope this gave you enough background to make something awesome. Explore, build, and let me know if this helped!


# Researcher
[1]. Adrian Sampson : http://www.cs.cornell.edu/~asampson/blog/llvm.html

[2]. Brandon Holt : http://bholt.org/posts/llvm-debugging.html

[3]. Chris Lattner's : http://nondot.org/sabre/Resume.html


# TIPS
RTFM : Read The Fucking Manual
GIYF : Google Is Your Friend
STFW : Search The Fucking Web
JFGI : Just Fucking Google It

# Reference
[1]. The LLVM Compiler Infrastructure : http://llvm.org/  
[2]. ** LLVM for Grad Students:** 
http://www.cs.cornell.edu/~asampson/blog/llvm.html  
[3]. Gentle introduction into compilers : 
https://blog.angularindepth.com/search?q=Gentle%20introduction%20into%20compilers   
[4]. 编程语言系列: https://riboseyim.github.io/2017/05/26/Language/
[5]. Quala: Type Qualifiers for LLVM/Clang : https://github.com/sampsyo/quala
[6]. LLVM Debugging Tips and Tricks : http://bholt.org/posts/llvm-debugging.html
[7]. Clang: a C language family frontend for LLVM : http://clang.llvm.org/
[8]. LLVM IR and Go : https://blog.gopheracademy.com/advent-2018/llvm-ir-and-go/