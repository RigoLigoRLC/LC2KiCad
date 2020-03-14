# LC2KiCad

![The LC2KiCad Logo. This Logo is licensed under CC-BY-SA 3.0 license.](./docs/LC2KiCad-Logo.svg)

[中文版本README](./docs/README.zh_CN.md)

---

## Overview

LC2KiCad is a software that is designed to be able to convert documents of EasyEDA (or aka. LCEDA, since it's owned by LCSC) to documents of KiCad 5.0 or higher. It is completely free, main part of code is licensed under GNU LGPL v3 license.

**LC2KiCad is currently in development, and many features weren't yet available.***

---

## How to compile the program

### Dependency

LC2KiCad requires C++ standard libraries to be present. Compilation process requires GCC and CMake. Other compilers were not tested. You will also need Git to be able to pull the repository (if required).

### Linux

```shell
git clone https://github.com/rigoligorlc/lc2kicad.git
cd lc2kicad
mkdir build && cd build
cmake .. && make
```

The compiled executable is right in `lc2kicad/build/`.

### Windows

You need to get Git, CMake and Mingw working, and make sure all of them are available in PATH. The commands are virtually the same as Linux.

```powershell
git clone https://github.com/rigoligorlc/lc2kicad.git
cd lc2kicad
mkdir build && cd build
cmake .. && mingw32-make
```
### macOS

No macOS compilation has been done.

---

## How to use LC2KiCad

### Important Notes!

- **LC2KiCad is available only in CLI (command-line interface) by design, and the author has no plan of adding a GUI.**

- **LC2KiCad command argument parser needs to be refactored, and everything listed here are subject to change. Program might not work as how this part described. NO WARRANTY IS GUARANTEED.**

  

- `lc2kicad`  Without an argument, the help message will be displayed.
- `lc2kicad -h` or `lc2kicad --help` Display the help message.
- `lc2kicad -V` or `lc2kicad --version` Display the version and about message.
- `lc2kicad FILE1 [FILE2] ...` Convert the files specified.
- **PLANNED BUT NOT IMPLEMENTED** *`lc2kicad -C[compatibility switches] FILE1 [FILE2] ...`* Convert the files with compatibility option(s). [Compatibility switches documentation](./docs/compatibility_switches.md)

LC2KiCad will open each file and try to parse each of them. If the parsing succeeded , LC2KiCad will try to write the converted content to a new file, with the same name (but not same extension name). If LC2KiCad cannot open a new file to write into, it will write everything into the standard output stream.

**To be filled with other information**

---

## Licensing

LC2KiCad core part is licensed under GNU Lesser General Public License v3.

LC2KiCad utilized RapidJSON libraries which is licensed under MIT License.

---

### Important Notes!

We **FORBID** the illegal use of converting others' files and libraries with the binary distribution of this program.

We **FORBID** the use that is outside the EasyEDA Terms of Use with the binary distribution of this program.

---

© Copyright RigoLigoRLC 2020.
