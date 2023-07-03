# The LLVM Target-Independent Code Generator


## 1. Target description 
- /llvm/Target/
- lib/Target/

### 1.1 TargetMachine Class  

### 1.2 DataLayout Class  

### 1.3 TargetLowering Class

### 1.4 TargetRegisterInfo & TargetRegisterClass

### 1.5 TargetInstrInfo Class 

### 1.6 TargetFrameLowering Class

### 1.7 TargetSubtarget Class


## 2. Machine code representation
- include/llvm/CodeGen/
- include/llvm/MC

- lib/CodeGen
- lib/MC

### 2.1 MachineFunction


### 2.2 MachineBasicBlock


### 2.3 MachineInstr


## 3. High-Level design of the code generator

### 3.1 Instruction Selection




#  Reference 
[1]. The LLVM Target-Independent Code Generator: https://llvm.org/docs/CodeGenerator.html#introduction-to-selectiondags    