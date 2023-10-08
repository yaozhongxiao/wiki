# lds

The job of the linker is to take all object files and libraries to be linked and create an executable. In order to do this, it must take all symbols from the object files, resolve unknown symbols on each input object file (this is, finding out which object file provides each missing symbol) and create a single output file with no unresolved symbols (except of those of dynamically linked libraries, which are resolved during runtime).

The goal of this article is to provide a brief introduction about the GNU linker script. With this article, you should able to understand how the GNU linker creates the executable file from the object files.

The Linker script consist of few parts below.
- Memory layout
- Entry point definition
- Section definitions

## Memory layout

The MEMORY command describes the location and size of memory blocks in the target. And for each linker script, it can only has at most one use of the MEMROY command, however, user can define as may memory regions within it.

The syntax of MEMORY command is below.
```
MEMORY
{
    name (attr) : ORIGIN = origin, LENGTH = len
    ...
}
```
```name```: defines the name of the regions that referred by the GNU linker.

```attr```: defines the attributes of a particular memory region. What the valid attribute should be made up of the options below (e.g. rwx).

| Letter | Section Attribute | 
|--------|-------------------|
|r| Read-Only sections |
|w| Read-Write sections |
|x| Sections contaning executable code |
|a| Allocated sections |
|i| Initialized sections |
|l| load attribute |
|!| Invert the sense of any of following attributes |

```ORIGIN```: specify the start address of the memory region in the physical memory.

```LENGTH```: specify the size of the memory region in bytes.

For example, the following GNU linker script defined two memory regions by using the MEMROY command. 
- The first region named as FLASH start at physical address 0x0 with 2048kB size. 
- And the second region named as RAM start as physical address 0x20000000 with 512kB size. 
The memory regions defined here corresponding to the whole Flash and RAM memory of the specified part number, read “Section definitions” in this article for how to use the memory regions.
```
MEMORY {
	FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x200000 /* 2048k */
	RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x80000 /* 512k */
}
```

how to create memory regions by using the MEMORY command, and then locate functions or entire file at the specific memory regions. Please refer to link below to get more information.

## Entry point
The ENTRY command is used for defining the first executable instruction. The syntax of ENTRY command is:
```
ENTRY(symbol)
```
What the argument of the command is a symbol name.

For example, the default following GNU linker script defined the first executable instruction of the target is the ‘Reset_Handler’.
```
ENTRY(Reset_Handler)
```
The symbol ‘Reset_Handler’ must be defined in code[1]. 

## Section definitions

The SECTIONS command controls how to map the input sections into output sections, and also the order of the output sections in memory. It can only has at most one SECTIONS command in a linker script file, however, many statements can be included within the SECTIONS command.

The most frequently used statement in the SECTIONS command is the section definition, which specifies the properties of an output section: its location, alignment, contents, fill pattern, and target memory region.

The full syntax of a SECTIONS command is:
```
SECTIONS {
...
secname start BLOCK(align) (NOLOAD) : AT(ldadr) [7]
  { contents } >region :phdr =fill
...
}
```

```secname```: The name of the output section.

```start```: Specify the address that the output section will be loaded at.

```BLOCK(align)```: Advance the location counter. prior to the beginning of the section, so the section will begin at the specified alignment.align is an expression.


```(NOLOAD)```: Mark a section to not be loaded at run time.

```AT ( ldadr )```: Specify the load address of the section to 'ldadr'. If don't use the AT keyword, the default load address of the section is same as the relocation address.

```>region```: Assign this section to a defined region of memory.

The ‘secname’ and ‘contents’ are required for a section definition, others are optional.

Multiple input sections in object files map to different output sections in the output file of the linker. The linker script takes care of specifying the memory layout of each output section. We define all output sections inside the SECTION command:
```
/* Define output sections */
SECTIONS
{
  .isr_vector : { /* This is the output section .isr_vector */
    exceptions.o (.isr_vector) /* This matches all .isr_vector sections in the exceptions.o input file */
  }
  .text : { /* This is the output section .text */
    *(.text) /* This matches all .text sections in all input files */   
    *(.text*) /* This matches all .text* sections in all input files */
  }
}
```

```exceptions.o(.isr_vector)``` input section contains 2 parts. They are input object file (exceptions.o) and the section in object file(.isr_vector). In contrast, ```*(.text)``` will contain all .text(.text) sections in all input files(*)[6].

> Input Section Syntax [8]
>> None of this syntax is used in practice but it’s useful to contextualize the syntax for pulling in a section. The full form of the syntax is
>> 
>> ```archive:object(section1 section2 ...)```
>> 
>> Naturally, all of this is optional, so you can write foo.o or libbar.a:(.text) or :baz.o(.text .data), where the last one means “not part of a library”. There’s even an EXCLUDE_FILE syntax for filtering by source object, and a INPUT_SECTION_FLAGS syntax for filtering by the presence of format-specific flags.
>> 
>> Do not use any of this. Just write *(.text) and don’t think about it too hard. The * is just a glob for all objects.




### example
Take the .text section definition in the default linker file as an example to illustrate how to define a section.
```
  .text :
  {
    KEEP(*(.vectors))
    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - __Vectors;
    __end__ = .;

    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    KEEP(*(.eh_frame*))
  } > FLASH
```

```.text``` is the name of the section, and the ```KEEP((*(.vectors))``` is used for marking the ‘.vectors’ input section not be eliminated, the similar method also applied to ‘.init’ ‘.fini’ and the ‘.eh_frame’ input sections. The special linker variable dot ```‘.’``` always contains the current output location counter, so it can get the end address of the vectors by using the dot ‘.’ variable following the KEEP((*(.vectors)), and calculate the size of the ‘.vectors’ input section.

And then place the ‘.ctors’ and ‘.dtors’ input section from the crtgebin.o and crtbegin?.o files into the .text output section.

The '.ctors' section is set aside for a list of constructors (also called initialization routines) that include functions to initialize data in the program when the program is started. And the '.dtors' is set aside for a list of destructors (also called termination routines) that should be called when the program terminates. For more information about the ‘.ctors’ and ‘.dtors’ sections, please refer to the link below[5].

```> FLASH``` assign the .text output section to the FLASH memory region that defined previously.


# Reference
[1]. Understand the GNU linker script of cortex M4: https://community.silabs.com/s/article/understand-the-gnu-linker-script-of-cortex-m4?language=en_US

[2]. Using a linker script in GCC to locate functions at specific memory regions: https://community.silabs.com/s/article/using-a-linker-script-in-gcc-to-locate-functions-at-specific-memory-regions?language=en_US

[3]. How to locate an entire file at specific memory region
: https://community.silabs.com/s/article/how-to-locate-an-entire-file-at-specific-memory-region?language=en_US

[4]. What's the default linker script of the MCU example project?: https://community.silabs.com/s/article/what-s-the-default-linker-script-of-the-mcu-example-project-x?language=en_US

[5]. How Initialization Functions Are Handled: https://gcc.gnu.org/onlinedocs/gccint/Initialization.html

[6]. Mastering the GNU linker script: https://allthingsembedded.com/post/2020-04-11-mastering-the-gnu-linker-script/

[7]. Command Language: https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html#SEC6

[8]. Everything You Never Wanted To Know About Linker Script:https://mcyoung.xyz/2021/06/01/linker-script/

[9]. An Introduction to Linker Files: Crafting Your Own for Embedded Projects:https://medium.com/@mkklyci/an-introduction-to-linker-files-crafting-your-own-for-embedded-projects-60ad17193229

[10]. The most thoroughly commented linker script (probably):
https://blog.thea.codes/the-most-thoroughly-commented-linker-script/