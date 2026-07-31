# Contributing Guideline
- Keep The Programming Language Syntax As Easier
- Use Optimized Runtime Instead Of Standard Runtime On Transpiler, And Sandbox
  But Runtime Is Located At `lib/runtime.asm`

## Contribute
Requred Dependencies:
- `cmake`: For Configure
- `python3`: For Higher-Level Build Automation
- `qt5`: For Argurement Parsing And Sending HTTP Request
- `llvm`: For Assembling/Compiling

Required Files Structure
```bash
root
|---docs # Documention Directory
|---include # Header Directory
    |------definations.h # Global Header
    |------lexer.h # Header Of Lexer
    |------parser.h # Header Of Parser
    |------miniaudio.h # CLI Audio Player Header
    |------preprocessor.h # Header Of Preprocessor/Optimizer
    |------runtime.h # Header Of Optimized Runtime
    |------transpiler.h # Header Of Transpiler/Translator
|---lib
    |--runtime.asm # Optimized Runtime
|---media # Audio Directory
|---scripts # Higher Level Scripts To Run
|---src # Main Source Directory
    |--main.cpp # Main Executeable Source Code
    |--lexer.cpp # Lexer
    |--parser.cpp # Parser
    |--preprocessor.cpp # Preprocessor/Optimizer
    |--transpiler.cpp # Transpiler/Translator
```

Building The Compiler:
```bash
cmake -B build
cd build
make -j $(nproc)
```