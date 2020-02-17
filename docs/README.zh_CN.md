# LC2KiCad

![LC2KiCad Logo。此Logo以CC-BY-SA 3.0协议授权。](./LC2KiCad-Logo.svg)

---

## 总览

LC2KiCad 是一个可以把EasyEDA的文档（或又称LCEDA，即立创EDA，因为该软件事实上为LCSC，即立创商城所有）转换为KiCad 5.0版本（或更高）文档的软件。该软件完全免费，它的主要部分在GNU LGPL v3协议授权下分发。

**LC2KiCad的主体目前还处于开发中，它的许多功能目前都不可用。***

---

## 编译本程序

### 依赖库

LC2KiCad需要使用C++的标准库组件。编译过程需要使用GCC和CMake。其他的编译器并未经过测试。如需克隆本仓库，Git也是需要使用的组件之一。

### Linux

```shell
git clone https://github.com/rigoligorlc/lc2kicad.git
cd lc2kicad
cmake . && make
```

编译出的二进制文件被存放在 `lc2kicad/build/`

### Windows

您需要让Git，CMake和MinGW正常工作，且确保它们的所在目录都加入了PATH环境变量。编译使用的命令和Linux基本相同。

```powershell
git clone https://github.com/rigoligorlc/lc2kicad.git
cd lc2kicad
cmake . && make
```
~~注意：Windows下用MinGW编译出的程序需要一些MinGW动态链接库才可以运行。您可以在KiCad的bin文件夹内找到这些运行库。~~

CMakeLists.txt已被调整。生成的程序已被静态链接，无需运行库。

### macOS

本程序没有在macOS下编译的文档。

---

## 如何使用LC2KiCad

### 重要信息！

- **根据LC2KiCad的设计，本程序只有CLI（命令行界面）可用，且原作者并没有加入GUI的意图。**

- **LC2KiCad的命令参数解析器需要重构，这里列出的操作随时可能变更。本程序也可能无法按照该部分的说明工作。本软件没有任何使用保障。**

  

- `lc2kicad`  不传递任何参数时，会显示帮助信息。
- `lc2kicad -h` 或 `lc2kicad --help` 显示帮助信息。
- `lc2kicad -V` 或 `lc2kicad --version` 显示该程序的版本号及关于信息。
- `lc2kicad 文件1 [文件2] ...` 转换指定的文件。
- **已计划但未实现** *`lc2kicad -C[兼容性选项] 文件1 [文件2] ...`* 使用指定的兼容性选项并转换指定的文件。

LC2KiCad会试图打开每个文件，然后解析所有的文件。 如解析成功，LC2KiCad会试图将转换出的内容写入新的文件，其文件名和源文件相同（但扩展名是KiCad文档的扩展名）。如果LC2KiCad无法打开一个可以写入的新文件，它会将所有的内容写入标准输出流。

**此部分仍待填写完整**

---

## 许可证

LC2KiCad的核心组件以GNU较宽松公共许可证（LGPL）v3授权。

LC2KiCad使用了RapidJSON，其许可证为MIT许可证。

---

### 重要信息！

我们**禁止**您使用该程序的二进制发行版本非法转换他人的文档。

我们**禁止**您在EasyEDA（立创EDA）用户使用协议允许的情形之外使用该程序的二进制发行版本。

---

© Copyright RigoLigoRLC 2020.
