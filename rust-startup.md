# Type 
## scalar -> webassembly
- bool(1 byte)
- char(4 bytes, unicode)
- i32/u32/i64/u64
- f32/f64

## Compound
### builtin type
- array: [type;size]
- tuple: (type,...)
### extend type
- struct (impl)
- enum ~ C++<enum + union> (impl)
- trait

## type attribute
- const
- mut
- & (borrow)
- pub

# Control flow
## condition
- if, if let (match)
- match (switch sematic)
## loop
- for/while/loop

# module structure
- workspace/package/(crate::mod)
- use mod::{item1, ..., item2}
- use mod::*
- x is mod with file name x.rs ?
- mod code.rs or mod {code} import code into self file ?


# key feature
## 解引用强制转换