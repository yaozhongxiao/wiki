# TableGen

TableGen source files contain two primary items: abstract records **(class)** and concrete records **(record)**. 

Classes and concrete records have a unique name, either chosen by the programmer or *generated* by TableGen. Associated with that name is a list of fields with values and an optional list of parent classes (sometimes called base or super classes). The fields are the primary data that backends will process.
> Note that TableGen assigns no meanings to fields; the meanings are entirely up to the backends and the programs that incorporate the output of those backends.

Both classes and concrete records can include fields that are uninitialized. The uninitialized “value” is represented by a question mark (?). Classes often have uninitialized fields that are expected to be filled in when those classes are inherited by concrete records. Even so, some fields of concrete records may remain uninitialized.

## 1. Type
The TableGen language is statically typed, using a simple but complete type system.    
Every value is required to have an associated type.
```
type identifer = value
```

TableGen has following kinds of types:
```
Type    ::=  "bit" | "int" | "string" | "dag"
            | "bits" "<" TokInteger ">"
            | "list" "<" Type ">"
            | ClassID
ClassID ::=  TokIdentifier
```
### 1.1 bit (bool)
A bit is a boolean value that can be 0 or 1.
```
bit bv = 0 | 1
```

### 1.2 int 
The int type represents a simple 64-bit integer value.  
Numeric literals take one of the following forms:
- Decimal 
    ```
    [+|-](0...9)+
    ```

- BinInteger 
    ```
    0b(0|1)+
    ```

- HexInteger 
    ```
    0x(0...9|'a'...'f'|'A'...'F')+
    ```

### 1.3 string
The string type represents an ordered sequence of characters of arbitrary length.  
TableGen has two kinds of string literals:
- TokString
    ```
    "non-characters and escapes"
    ```

- TokCode
    ```
    [{ (shortest text not containing }]) }]
    ```
A TokCode is nothing more than a multi-line string literal delimited by [{ and }]. It can break across lines and the line breaks are retained in the string.

### 1.4 bits<n> (bit array)
The bits type is a fixed-sized integer of arbitrary length n that is treated as separate bits.  
The bits of the field can be set individually or as subfields
```
bits bv_a =  { 0, 0, 1, 0 };
bv_a[1..3] = 0b101
bits bv_b = bsv[1...5]
```

### 1.5 list<type>
This type represents a list whose elements are of the type specified in angle brackets. The element type is arbitrary; it can even be another list type. List elements are indexed from 0.
```
list<dag> dags_a = [(add R0, R2), (add R1, R3)];
list<dag> dags_b = dags_a[0,1];
```

### 1.6 dag
This type represents a nestable directed acyclic graph (DAG) of nodes. Each node has an operator and zero or more arguments (or operands). An argument can be another dag object, allowing an arbitrary tree of nodes and edges.   
The syntax of a dag instance is:
```
( operator argument1, argument2, … )
```

The operator must be present and must be a **record**. There can be zero or more arguments, separated by commas. The operator and arguments can have three formats.
| Format     | Meaning |
|------------| --------|
|value       | argument value |
|value:$name | argument value and associated name |
|$name       | argument name with unset (uninitialized) value |

The **value** can be any TableGen value. The **name**, if present, must be a TokVarName, which starts with a dollar sign ($). The purpose of a name is to tag an operator or argument in a DAG with a particular meaning, or to associate an argument in one DAG with a like-named argument in another DAG.

### 1.7 ClassID (record)
Specifying a class name in a type context indicates that the type of the defined value must be a subclass of the specified class. This is useful in conjunction with the list type;
The ClassID must name a class that has been previously declared or defined.

```
class Register {}
list<Register> rl;
```

## 2. Value Expressions
## 2.1 Identifier
TableGen has name- and identifier-like tokens, which are case-sensitive.

- TokIdentifier
    ```
    ("0"..."9")* (alpha) (ualpha | "0"..."9")*
    ```

- TokVarName (dag name)
    ```
      $(alpha)(ualpha |  "0"..."9")*
    ```

### 2.2 uninitialized value
```
simple_value_uninitialized = ?
```

### 2.3 bool value
```
bit b_va = true | false | 0 | 1
```

### 2.4 int value
```
int v = 0b10011 | 0xabdf00 | 6

bit = v{17}  // The final value is bit 17 of the integer value (note the braces).

bits<8> = v{8...15}  // The final value is bits 8–15 of the integer value. The order of the bits can be reversed by specifying {15...8}.


```
### 2.5 string
```
string str = "string token"
string code = [{ dag v:$v1 r:$v2 }]
```
### 2.6 bits<n>
```
bits<n> b_va = val{ 1, 3, 5...8 }
```
### 2.7 list<T>
```
list<int> l_v = [ v1, v2, v3 ]

list<T> slice_a = l_v[i,]
list<T> slice_b = l_v[i,j]
```
### 2.8 dag
```
dag input = ( operator argument1, argument2, … )

Type v = input.argument1  // value.field
```

### 2.9 class | multiclass
```
class classId <arg1=v1, arg2> {
}

multiclass classId<arg1=v1, arg2> {
}

> This form creates a new anonymous record definition (as would be created by an unnamed def inheriting from the given class with the given template arguments; 

classId c_a = classId<val1, val2>;
def : classId<val1, val2>;
```

## 3. Statement
The following statements may appear at the top level of TableGen source files.
```
TableGenFile ::=  (Statement | IncludeDirective
                 | PreprocessorDirective)*
Statement    ::=  Assert | Class | Def | Defm | Defset | Defvar
                 | Foreach | If | Let | MultiClass
```

### 3.1 class
A class statement defines an abstract record class from which other classes and records can inherit.
```
class classID <Type Arg1 [= val], ...> : parantClass<PosVal, ... , NameArg = Val, ...> {
    expression;
    let id_vb{range} = value1;
    defvar id_va = Val;
    Assert expr
}
```

#### 3.1.1 Class Declaration
As shown above, A class can be parameterized by a list of “template arguments,” whose values can be used in the class’s record body. These template arguments are specified each time the class is inherited by another class or record.
If a template argument is not assigned a default value with =, it is uninitialized (has the “value” ?) and must be specified in the template argument list when the class is inherited (required argument). If an argument is assigned a default value, then it need not be specified in the argument list (optional argument). In the declaration, all required template arguments must precede any optional arguments. The template argument default values are evaluated from left to right.

```
class classID <TypeA Arg1, TypeB Arg2 = val2, ...> {
    TypeA id = Arg1
    let var = Arg1
}
```
#### 3.1.2 Parent Class.
class declaration can inherit from parent classes. It can include a list of parent classes from which the current class inherits. When a class C inherits from another class D, the fields of D are effectively merged into the fields of C.

```
class C <TypeA Arg1, TypeB Arg2 = val2, ...> : D <Arg1, FieldA = Arg2> {
    TypeA id = Arg1
    let var = Arg1
}
``````

#### 3.1.3 Class Body
class will define field and other statements in body.

``` 
calss ClassID ... {
    expression;
    let id_vb{range} = value1;
    defvar id_va = Val;
    Assert expr
}
```
Every class has an implicit template argument named NAME (uppercase), which is bound to the name of the Def or Defm inheriting from the class. If the class is inherited by an anonymous record, the name is unspecified but globally unique.

A class can be invoked in an expression and passed template arguments.
This causes TableGen to create a new anonymous record inheriting from that class.
the record receives all the fields defined in the class.Those fields can then be retrieved in the expression invoking the class as follows employed as a simple subroutine facility.
```
int Result = ... CalcValue<arg>.ret ...;
```

### 3.2 def
The record define the main body of the definition, which contains the specification of the fields of record.
```
def [ RecordName ] : parantClass<PosVal0, PosVal1, ..., NameArg = Val, ...> {
    expression;
    let id_vb{range} = value1;
    defvar id_va = Val;
    Assert expr
}
```

A field definition in the body specifies a field to be included in the class or record. If no initial value is specified, then the field’s value is uninitialized. The **type must be specified**; TableGen will not infer it from the value. The keyword code may be used to emphasize that the field has a string value that is code.
- The ```let``` form is used to reset a field to a new value
- The ```defvar``` form defines a variable whose value can be used in other value expressions within the body. The variable is not a field: it does not become a field of the class or record being defined. Variables are provided to hold temporary values while processing the body

The following steps are taken by TableGen when a record is built [2]. Classes are simply abstract records and so go through the same steps.
1. ```(1). Build the record name (NameValue) and create an empty record.```
2. Parse the parent classes in the ParentClassList from left to right, visiting each parent class’s ancestor classes from top to bottom.
    - ```(2). Add the fields from the parent class to the record.```
    - ```(3). Substitute the template arguments into those fields.```
    -  Add the parent class to the record’s list of inherited classes.
3. ```(4). Apply any top-level let bindings to the record. Recall that top-level bindings only apply to **inherited fields**.```

4. Parse the body of the record.
    - ```(5). Add any fields to the record.```
    - ```(6). Modify the values of fields according to local let statements.```
    - ```(7). Define any defvar variables.```

5. ```(8). Make a pass over all the fields to resolve any inter-field references.```
6. Add the record to the final record list.

### 3.3 multiclass
multiclasses allow a convenient method for defining many records at once(works as a namespace). You can think of a multiclass as a macro or template that expands into multiple records.
```
multiclass classID<type arg1, type arg2 = val2 ...> : parentClass<val1, name2=val2 ...> {
    let ...
    defvar ...
    def ...
    defm ...
    if ...
    foreach ...
    assert  ...
}
```
A multiclass can inherit from other **multiclasses**, which causes the other multiclasses to be expanded and contribute to the record definitions in the inheriting multiclass.

Also as with regular classes, the multiclass has the implicit template argument NAME (see NAME). When a named (non-anonymous) record is defined in a multiclass and the record’s name does not include a use of the template argument NAME, such a use is automatically prepended to the name. That is, the following are equivalent inside a multiclass:
```
muticlass clz {
    def Foo ...
    def NAME # Foo ...
}
```

### 3.4 defm
```defm``` statement “invoke” them and process the multiple record definitions in those multiclasses. Those record definitions are specified by def statements in the multiclasses, and indirectly by defm statements.
```
defm    : SomeMultiClass<...>;   // A globally unique name.
defm "" : SomeMultiClass<...>;   // An empty name.
defm [ name ] : SomeMultiClass<...>; 
```
**Note that the defm does not have a body**

This statement instantiates all the records defined in all the specified multiclasses, either directly by def statements or indirectly by defm statements. These records also receive the fields defined in any **regular classes** included in the parent class list. This is useful for adding a common set of fields to all the records created by the defm.
```
defm : SomeMultiClass<...>, RegularClass<...>;
```

### 3.5 defset
The defset statement is used to collect a set of records into a global list of records.
```
defset list<class> setname  = {
    statement
}
```

### 3.6 defvar
A defvar statement defines a global variable. Its value can be used throughout the statements that follow the definition.
The type of the variable is automatically inferred.
Once a variable has been defined, it cannot be set to another value.
In addition to defining global variables, the defvar statement can be used inside the Body of a class or record definition to define local variables. 
```
defvar varname = value;
```

### 3.7 let
A let statement collects a set of field values (sometimes called bindings) and applies them to all the classes and records defined by statements within the scope of the let.
Top-level let statements are often useful when a few fields need to be overridden in several records.(override a single filed can be resolved by redefine(let) in record directly)
```
let name1=val1, name2=val2 ... in {
    def : class <args ...> {}
}
```
The let statement establishes a scope, which is a sequence of statements in braces or a single statement with no braces. The bindings in the LetList apply to the statements in that scope. The let acts to override inherited field values. A let cannot override the value of a template argument.

### 3.8 if
The if statement allows one of two statement groups to be selected based on the value of an expression.
FIXME(): only work as global statement and multiclass statement, there are errors in class and record body!
```
if value then {
    statement ...
} else {
    statement ...
}
```

### 3.9 foreach
The foreach statement iterates over a series of statements, varying a variable over a sequence of values.
```
    foreach id = [...] in {
        statement ...
    }
```
```
foreach i = [0, 1, 2, 3] in {
  def R#i : Register<...>;
  def F#i : Register<...>;
}
```

### 3.10 assert
The assert statement checks a boolean condition to be sure that it is true and prints an error message if it is not.
```
assert condition, message
```

## 4. Preprocessor
### 4.1 include
TableGen has an include mechanism. The content of the included file lexically replaces the include directive and is then parsed as if it was originally in the main file.
```
include "TokeString.td"
```

### 4.2 preprocessor directives
source files and included files can be conditionalized using preprocessor directives.
```
#define DIRECTIVES

#ifdef DIRECTIVE_A
#else
#endif

#ifndef DIRECTIVE_B
#else
#endif
```
A MacroName may be defined externally using the -D option on the *-tblgen command line:
```
llvm-tblgen self-reference.td -Dmacro1 -Dmacro3
```


## 5. Extension
TableGen provides “bang operators” [3], Bang operators act as functions in value expressions.

```
BangOperator ::=  one of
                  !add         !and         !cast        !con         !dag
                  !div         !empty       !eq          !exists      !filter
                  !find        !foldl       !foreach     !ge          !getdagarg
                  !getdagname  !getdagop    !gt          !head        !if
                  !interleave  !isa         !le          !listconcat  !listremove
                  !listsplat   !logtwo      !lt          !mul         !ne
                  !not         !or          !range       !repr        !setdagarg
                  !setdagname  !setdagop    !shl         !size        !sra
                  !srl         !strconcat   !sub         !subst       !substr
                  !tail        !tolower     !toupper     !xor
```

```
CondOperator ::=  !cond(cond1:value1, cond2:value2, ....)
```


## 6. Reference 
[1]. TableGen Programmer’s Reference: https://llvm.org/docs/TableGen/ProgRef.html#directed-acyclic-graphs-dags
[2]. How records are built: https://llvm.org/docs/TableGen/ProgRef.html#how-records-are-built
[3]. https://llvm.org/docs/TableGen/ProgRef.html#appendix-a-bang-operators