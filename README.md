
# toolbox

A set of simple and convenient `.c` files that is usable within a `CRTL+C` + `CTRL+V` + `#include`.
  + [Overview](#overview)
  + [Compatibility](#Compatibility)
  + [Getting Started](#getting-started)
  + [Examples](#examples)
  + [Reference](#reference)
    + [shared.c](#shared.c)
    + [platform.c](#platform.c)

## Overview

This is a collection of code that I commonly use or write. It is organized into a collection of files with each file providing a set of functionality. Certain files can depend on other files as well (specified with `#include` directives).

The core motivation of this project was to write a set of core functionality that I find to be essential or just plain useful to have before starting a new project.

Here is "**[examples/hello_world.c](examples/hello_world.c)**", which I think is a nice illustration of what this project was intended for:

```c
#include "../shared.c"
#include "../platform.c"
#include "../print.c"

local s32 Main(main_info* Info)
{
    Println(StdOut(), Str("Hello, world!"));
    return (0);
}
```

This codebase has a number of features that I think is desirable:
  + **Zero dependencies**: This code base doesn't use the standard library or any other third-party library. Everything was implemented from scratch, and certain files can be integrated into a low-level environment (operating systems, embedded, ...) without much difficulty.
  + **Small and Convenient**: Each file exposes a small, convenient API along with the implementation of said API. Most files start with a list of dependencies on other files and a struct + function cheatsheet, which serves as a convenient reference.
  + **Full Access to Source Code**: Including the `.c` files means building the source code of the files along with the program. This means that debugging becomes infinitely easier as you have complete access to the internals, and are able to place debug breakpoints along with watches on certain values.
  + **Portable**: Certain files like `platform.c` may depend on the underlying operating system or the instruction set architecture (ISA) of the machine. Several `#if` checks are placed throughout, and will notify the programmer if an operating system or an ISA hasn't been implemented yet. This makes it easy to identify parts of the codebase for porting.

## Compatibility

As of the time of this writing, this codebase was developed and tested on Linux kernel version `7.1.11-arch1-1` with the `clang` compiler. Windows and MSVC `cl` compiler support are both scheduled, and will be implemented next.

<center>

| Symbol | Meaning          |
| ------ | ---------------- |
| ✅     | Full support     |
| 🚧     | Being developed  |
| ❌     | No support yet   |
| ❓     | Not available    |

| Operaing System / Architecture | **x86_64** | **ARM64** | **RISCV64** |
| ------------------------------ | ---------  | --------- | ----------- |
| **Windows**                    |     🚧     |    ❌     |     ❓      |
| **Linux**                      |     ✅     |    ❌     |     ❌      |
| **MacOS**                      |     ❌     |    ❌     |     ❓      |

| Compiler                  | Status |
| ------------------------- | ------- |
| **clang**                 |    ✅   |
| **clang-cl**              |    🚧   |
| **cl**                    |    🚧   |
| **gcc**                   |    ✅   |

</center>

## Getting Started

***Get the code***

A simple `git clone` will provide everything you need:

```
> git clone https://github.com/vakye/toolbox.git
```


***Write your `main.c`***

Including `shared.c`, `platform.c` and writing your `Main()` is the bare essential. Here is an example `main.c` that you can write:

```c
#include "shared.c"
#include "platform.c"

local s32 Main(main_info* Info)
{
  return (0);
}
```

To make things more exciting, we can include `print.c` and extend our `Main` function to print our favorite message to the console:

```c
#include "shared.c"
#include "platform.c"
#include "print.c"

local s32 Main(main_info* Info)
{
  Println(StdOut(), Str("Hello, world!\n"));
  return (0);
}
```

***Compiling with `clang`***

Here is a list of necessary flags and the build command for `clang`:
  + `-ffreestanding`: Tell the compiler to not emit calls to C standard library functions.
  + `-fno-stack-protector`: Tell the compiler to not emit `__stack_chk_fail`, which is provided by the C standard library.
  + `-std=c11`: Specify C11 standard. This codebase uses a couple of C11 features.
  + `-nostdlib`: Don't compile with the standard library.
  + `-fuse=ld=lld`: Use the `lld` linker.
  + `-Wl,-nostdlib`: Tell the linker to not link with the standard library.
  + `-Wl,-entry,EntryPoint`: Tell the linker that our entry point is the `EntryPoint` function provided by `platform.c`

```
> clang -ffreestanding -fno-stack-protector -std=c11 -nostdlib -fuse-ld=lld -Wl,-nostdlib -Wl,-entry,EntryPoint main.c -o main
```

This should produce an executable named `main`.

***Running***

Once compilation has finished successfully, we can run the executable to get our favorite message:

```
> ./main
Hello, world!
```

## Examples

---

### Build Instructions

**Linux**: requires `clang` to be installed
```
> ./build_examples.sh
[SUCCESS]: ...
[SUCCESS]: hello_world.c
[SUCCESS]: simple_echo.c
[SUCCESS]: ...
```

---

+ [`hello_world.c`](examples/hello_world.c): Prints `Hello, world!` to the console.
  ```
  > ./build/hello_world
  Hello, world!
  ```

+ [`simple_echo.c`](examples/simple_echo.c): Prints command line arguments to the console.
  ```
  > ./build/simple_echo This is a basic clone of the echo command.
  This is a basic clone of the echo command.
  ```

+ [`text_stat.c`](examples/text_stat.c): Receives a text file path as a command line argument and prints out text statistics (character count, word count, ...).
  ```
  > ./build/simple_echo "Some random text! 123123" > my_text_file.txt
  > ./build/text_stat my_text_file.txt
  Text statistics:
    Word count:        3
    Character count:   14
    Whitespace count:  5
    Digit count:       6
    Punctuation count: 1
  ```

## Reference

---

### shared.c
+ Dependencies: `None`

---

***Compiler Detection***

| `#define`       | Values       |
| --------------  | ------------ |
| `CompilerClang` | `1` if the `clang` compiler is being used, `0` otherwise. |
| `CompilerGCC`   | `1` if the `gcc` compiler is being used, `0` otherwise.   |
| `CompilerMSVC`  | `1` if the `cl` compiler is being used, `0` otherwise.    |

---

***Operating System Detection***

| `#define`         | Values       |
| ----------------- | ------------ |
| `PlatformWindows` |  `1` if compilation target platform is `Windows`, `0` otherwise. |
| `PlatformLinux`   |  `1` if compilation target platform is `Linux`, `0` otherwise.   |

---

***Architecture Detection***

| `#define`           | Values       |
| ------------------- | ------------ |
| `ArchitectureX64`   |  `1` if compilation target architecture is `x86_64`, `0` otherwise. |
| `ArchitectureARM64` |  `1` if compilation target architecture is `ARM64`, `0` otherwise. |
| `ArchitectureRV64`  |  `1` if compilation target architecture is `RISVC64`, `0` otherwise. |

---

***Keywords***

| `#define`           | Value       | Comment |
| ------------------- | ----------- | ------- |
| `local`             | `static`    | Marks a function or global variable as being private to this executable/object file. This should be used for almost every function or global variable as it may allow the compiler to freely remove unused code/data. Furthermore, it may allow for inline optimizations. |
| `persist`           | `static`    | Marks a variable inside a function as having persistent storage, meaning that it retains its value even after the function has returned and gets called again. Usually, this is used for large buffers inside functions so that the compiler doesn't have to allocate a large region of memory on the stack for every function call. |

---

***Macros***

| `#define`                     | Comment |
| ----------------------------- | ------- |
| `CTAssert(Expression)`        | Performs a compile-time assertion on `Expression`. If `Expression` evaluates false, then the compiler will report it as an error. Else, the compiler will carry on as usual. |
| `ArrayCount(Array)`           | Compute the number of elements in an array that was declared with a constant size. For example, `int MyArray[4]` will lead to `ArrayCount(MyArray) = 4`. Please note that this only works for constant-size arrays that are declared as variables, it does not work for constant-size arrays declared as function arguments. |
| `OffsetOf(Structure, Member)` | Compute the byte offset of a `Member` inside a `Structure` type. An example of this is `struct my_struct { int A, B; };` will lead to `OffsetOf(my_struct, B) = 4`. |
| `Minimum(A, B)`               | Returns `A` if `A` is smaller than `B`, else returns `B` |
| `Maximum(A, B)`               | Returns `A` if `A` is larger than `B`, else returns| `B` |
| `AlignDown(Value, PowerOf2)`  | Rounds `Value` to the nearest multiple of `PowerOf2` that is smaller than `Value`. |
| `AlignUp(Value, PowerOf2)`    | Rounds `Value` to the nearest multiple of `PowerOf2` that is larger than `Value`. |
| `KB(Amount)`                  | Returns the amount of bytes for `Amount` kibibytes (2^10 bytes). |
| `MB(Amount)`                  | Returns the amount of bytes for `Amount` mibibytes (2^20 bytes). |
| `GB(Amount)`                  | Returns the amount of bytes for `Amount` gibibytes (2^30 bytes). |
| `TB(Amount)`                  | Returns the amount of bytes for `Amount` tibibytes (2^40 bytes). |

---

***Types***

| `typedef`                     | Comment |
| ----------------------------- | ------- |
| `s8`                          | Signed 8-bit integer type |
| `s16`                         | Signed 16-bit integer type |
| `s32`                         | Signed 32-bit integer type |
| `s64`                         | Signed 64-bit integer type |
| `u8`                          | Unsigned 8-bit integer type |
| `u16`                         | Unsigned 16-bit integer type |
| `u32`                         | Unsigned 32-bit integer type |
| `u64`                         | Unsigned 64-bit integer type |
| `ssize`                       | Largest signed integer type supported by the architecture |
| `usize`                       | Largest unsigned integer type supported by the architecture |
| `f32`                         | 32-bit floating point type |
| `f64`                         | 64-bit floating point type |
| `b8`                          | 8-bit boolean type |
| `b16`                         | 16-bit boolean type |
| `b32`                         | 32-bit boolean type |
| `b64`                         | 64-bit boolean type |

---

***Constants***

| `#define`                     | Value                    | Comment |
| ----------------------------- | ------------------------ | ------- |
| `true`                        | `1`                      | Represents a boolean true value |
| `false`                       | `0`                      | Represents a boolean false value |
| `S8Min`                       | `-128`                   | The minimum signed 8-bit integer limit |
| `S16Min`                      | `-32768`                 | The minimum signed 16-bit integer limit |
| `S32Min`                      | `-2147483648`            | The minimum signed 32-bit integer limit |
| `S64Min`                      | `-9223372036854775808`   | The minimum signed 64-bit integer limit |
| `S8Max`                       | `+127`                   | The maximum signed 8-bit integer limit |
| `S16Max`                      | `+32767`                 | The maximum signed 16-bit integer limit |
| `S32Max`                      | `+2147483647`            | The maximum signed 32-bit integer limit |
| `S64Max`                      | `+9223372036854775807`   | The maximum signed 64-bit integer limit |
| `U8Max`                       | `+255`                   | The maximum unsigned 8-bit integer limit |
| `U16Max`                      | `+65535`                 | The maximum unsigned 16-bit integer limit |
| `U32Max`                      | `+4294967295`            | The maximum unsigned 32-bit integer limit |
| `U64Max`                      | `+18446744073709551615`  | The maximum unsigned 64-bit integer limit |
| `SSizeMin`                    | Depends on architecture. | The minimum limit for the largest signed integer supported by the architecture |
| `SSizeMax`                    | Depends on architecture. | The maximum limit for the largest signed integer supported by the architecture |
| `USizeMax`                    | Depends on architecture. | The maximum limit for the largest unsigned integer supported by the architecture |

---

***Memory***

| Function                                                                | Comment |
| ----------------------------------------------------------------------- | ------- |
| `local void ZeroMemory(void* DestInit, usize Size)`                     | For `Size` bytes, set every byte in the memory region starting from `DestInit` to `0`. |
| `local void FillMemory(void* DestInit, u8 Byte, usize Size)`            | For `Size` bytes, set every byte in the memory region starting from `DestInit` to `Byte`. |
| `local void CopyMemory(void* DestInit, void* SourceInit, usize Size)`   | Copies `Size` bytes from the memory region starting at `SourceInit` to the memory region starting at `DestInit` |

| #define                                                                 | Comment |
| ----------------------------------------------------------------------- | ------- |
| `ZeroType(Pointer)`                                                     | Zeroes the memory region starting at `Pointer` for `sizeof(*Pointer)` bytes. |
| `ZeroArray(Pointer, Count)`                                             | Zeroes the memory region starting at `Pointer` for `sizeof(*Pointer) * (Count)` bytes. |

---

***String***

| `struct string`  | Comment |
| ---------------- | ------- |
| `char* Data`     | A pointer to the very first byte of the string. Not expected to be null-terminated. |
| `usize Size`     | Number of bytes within the string, not including null terminator if string is null-terminated. |

| #define                                                                 | Comment |
| ----------------------------------------------------------------------- | ------- |
| `Str(Literal)`                                                          | Creates a `string` struct from a string literal. An example is `Str("Hello") = (string){.Data = "Hello", .Size = 5}` |
| `StrData(Data, Size)`                                                   | Creates a `string` struct a pointer to the first byte of the string `Data`, and the number of bytes within the string `Size`. |

| Function                                                                | Comment |
| ----------------------------------------------------------------------- | ------- |
| `local string CString(char* Data)`                                      | Creates a `string` struct from a null-terminated string starting at `Data` |

---

### platform.c

