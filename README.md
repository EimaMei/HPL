<p align="center">
    <img src="images/logo.png?raw=true" alt="HPL logo" style="width:60%">
    <br/>The modern, developer solution to HOI4 modding.<br/>
    <br/>
    <a href="https://github.com/EimaMei/HPL/tree/main">
        <img src="https://img.shields.io/github/license/EimaMei/HPL">
    </a>
    <a href="https://discord.gg/T5R74EhQtB">
        <img src="https://img.shields.io/discord/1057613941355839518?logo=discord">
    </a>
    <br/>
</p>

# What is HPL?
The HOI4 Programming Language (HPL) is a domain-specific interpreter programming language made for sane HOI4 modding development. Its main goal is to substantially improve the current HOI4 modding experience by reworking HOI4's syntax to be much more user-friendly syntax and providing new features for more efficient and faster development, all while giving the user plenty room for customizability.

# Why HPL?
One of the things that the HOI4 modding scene suffers from is the low supply of HOI4 coders who are skilful with the Clausewitz language. Reason being that while it's simple to learn, it's really tedious to use and debug for both simple and complex parts of modding. What also doesn't help is that it lacks a bunch of simple programming language features that would improve the experience of developers so much.

People tried improving the developer experience by creating CLI and GUI tools to do everything for developers, however more often than not those projects either fail due to a multitude of reasons, are too limiting or nothing of worth. However, creating a proper language for HOI4 modding is one of the best solutions to this problem, as a language like HPL can give a lot of options to developers with how they want to solve their HOI4 modding related problems while being updated constantly. You can use HPL to create a bunch of small yet effective scripts, or even create entire developer environments.

HPL will not only just be focusing on achieving full HOI4 modding compatibility, but also to optimize HOI4 modding itself. Instead of needing tens of lines of code for a simple event (or even more), HPL will be able to provide the same result but with dramatically less lines of codes, improving the dev experience dramatically in the process. HPL even reduces how many characters and words you have to write per line with this.

# Examples
[general](/examples/general/main.hpl) - Shows the general programming features of HPL and what you can do with it.

[country](/examples/country/main.hpl) - Creates a nation.

[event](/examples/event/main.hpl) - An example of creating simple HOI4 events.

# Feature set of HPL
A quick overview of most features in HPL:
- Create and edit variables.
- Main types (`string`, `int`, `float`, `bool`, `scope`) and generic type `auto`.
- A VSCode extension which provides syntax highlighting, auto-code fillings and other useful features (click [here](README.md#hpl-visual-studio-code-extension) for more info).
- Plenty of core functions to help with hoi4modding, such as functions that help with converting images to `.dds`, resizing images, creating/removing files and folders etc.
- Standard libraries for creating HOI4 mods.
- HOI4 scopes COMPLETELY reworked to make it easier to use for devs (dubbed HOI4 Scripting+ Mode - HSM).
- Declare functions with return types.
- Execute functions and get their returns.
- Out of order function parameters.
- `if` statements.
- Python's `f-string`.
- C's `+` to combine strings.
- C-based structures.
- Debug and logging modes.
- Simple math (`++`, `--`, `+=`, `-=`, `=`, `/=`, `%=`).
- Handles all of the localisation for you.

# Status on the implementation of HOI4 modding
- [x] `descriptor.mod` creation.
- [ ] Event creation (Practically done).
- [ ] Nation creation (Somewhat started).
- [ ] Proper localisation support for other languages.
- [ ] Decision modding.
- [ ] Focus tree modding.

# Building and installing HPL
**Note:** Currently the main targetted platform for HPL is Windows 10 and up. Due to this, the Unix versions of HPL (especially the Linux builds) may contain bugs or be unstable. Also, only `x86` binaries for Windows, MacOS and Linux will be provided and supported.

## A list of requirements for installation:
- C++20
- Clang (Apple Clang on MacOS)
- Make

## Steps to build:
```bash
git clone https://github.com/EimaMei/HPL/tree/canary.git
cd HPL
make
```

## Installation
**For now only Windows has an installer.**

To install HPL on Windows, all you have to do is run the `installer.ps1` script in PowerShell once and it'll setup everything for you

# HPL Visual Studio Code extension
To make development with HPL much more colourful, fun and modern, programmer [Allyedge](https://github.com/Allyedge) has created a VSC extension for programming in HPL. For now it only has a syntax highlighter and a few basic language support features, however in the future it's gonna have much more features to make development even easier (think of it like the C/C++ VSC extension).

To download it you can just look up 'HPL' in the marketplace and download the first result, or go to [this link](https://marketplace.visualstudio.com/items?itemName=Allyedge.hpl).

# Credits
[SOIL2 (forked version)](https://github.com/EimaMei/SIL2) - for the `convertToDds` core function (HPL uses a modified version of SOIL2 to remove unneeded OpenGL requirements).

[Allyedge](https://github.com/Allyedge) - for creating the HPL VSC extension.
