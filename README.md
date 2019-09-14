# include_guard

A C/C++ include guard generator.

## How it works

Generates an include guard with the following pattern and writes it into each given file:

```cpp
#ifndef FILENAME_H
#define FILENAME_H
#endif /* !FILENAME_H */
```

For the generation of the macro name, each alphanumeric character gets converted to uppercase and all special characters get converted to underscores.

## Usage

`include_guard FILES...`
Be aware that all files will get truncated.

