Toy scripting language being made for funsies. Very work in progress.

## Building

On Windows, use [MSYS2](https://www.msys2.org/) or [WSL](https://learn.microsoft.com/en-us/windows/wsl).

Dependencies:
- Python 3 or later
- GCC 10+ or Clang 10+

To build, invoke `build.py` using a python interpreter.

The following environment variables can be set to customise the build:
- `CPP_COMPILER` - Path to the specific C++ compiler to use (default `g++`)
- `DEBUG` - Set to `1` to disable optimisations and export debug symbols (default `0`)

Example:
```
CPP_COMPILER=clang++ DEBUG=1 python3 build.py
```

## Using

After building, the interpreter `scri` (`scri.exe` on Windows) is in the directory `gen`.

The command line interface is:
```
scri [input file(s)]
```
