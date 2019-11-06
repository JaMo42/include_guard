# include_guard

[![CodeFactor](https://www.codefactor.io/repository/github/jamo42/include_guard/badge)](https://www.codefactor.io/repository/github/jamo42/include_guard)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/2ba7fd40538246e382094aa8b4c1c2d4)](https://www.codacy.com/manual/JaMo42/include_guard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=JaMo42/include_guard&amp;utm_campaign=Badge_Grade)

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

This program does not take any options, all arguments are interpreted as file names.
