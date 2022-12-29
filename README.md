# What is HPL?
The HOI4 Programming Language (HPL) is a high-level interpreter programming language for HOI4 that is set to make HOI4 modding much better, while also adding a bunch of new features to make HOI4 modding more enjoyable.

# Why HPL?
Because the current HOI4 scripting "language" lacks very basic programming features (which makes the code more ugly and longer), overcomplicates things for the sake of simplicity, always breaks when a new DLC launches and is just generally not fun to program in. This is why I always refer to the current HOI4 coding as "scripting", as its very basic and lacks the many features that programmer like myself take for granted.

The solution to this? Create an actual programming language designed for HOI4 coders. HPL will attempt to fix all of these issues by doing the following:
- Implement basic language features that many modern languages have (structures, loops etc).
- Rework the syntax of the HOI4 scripting language to make it more C-based.
- Make the elements of HOI4 modding easier to implement (automatically converting images to `.dds`, resizing flags to the correct size and format, make `if` statements and variables behave like how they do in proper languages, and so on)
- Have proper error checking when building code so that mods don't have thousands of errors/crash at startup.
- Enforcing stability so that code doesn't break each time a new DLC releases.

# Current status of HPL
Currently HPL is ***very*** expiremental, as such bugs should be expected and reported. However it now already has quite a few basic language features to toy around with, as well as the ability to use `libpdx.hpl` to ouput a few pieces of HOI4 code. With enough time (months, maybe a year or two?) it'll be stable enough for normal use cases for mods (hell, maybe even for full-scale productions but I don't expect that anytime soon, if at all).
# Current roadmap
## Step 1: Building the base code
Currently the language is barely done and it'll take awhile before any random modder will be able to use it without encountering some bug/unimplemented HOI4 functions. As such, these basic features must be implemented:

### Important features to add:
- [X] Save variables in memory.
- [X] Being able to edit variables at runtime.
- [X] Finish basic structures.
- [X] Access a struct's member and also edit it.
- [X] Save a function (and its arguments) in memory, while also retaining its code.
- [X] Delete variables when they're out of scope.
- [X] Execute functions when they're used.
- [X] Add returns to core typed functions.
- [X] Add f-string.
- [X] Being able to set the value of a __CORE TYPED__ variable from a function's return.
- [X] Add returns to non-core typed functions.
- [X] Being able to set the value of a __STRUCT TYPED__ variable from a function's return (ties in with "Add returns to non-core typed functions").
- [X] More debug options.
- [X] `if` statement
- [ ] `else` and `else if` statements.

### Not as important features to add:
- [ ] Add more runtime errors (defining variables/functions that already exist, too many curly brackets etc.)
- [ ] Make error/warnings reports more pretty (more aline with how GCC does it).
- [ ] Multiline variables
- [X] Finalize how scopes work in general.
- [ ] Out of order initializations of struct variables.
- [ ] Conditional operator (<condition> ? <true> : <false>)

## Step 2: Core functions implementation
When the base code for the interpreter is done, it'll allow us to finally implement the core functions of the language. Without core functions, we won't be able to build HOI4 code. Some of these functions will allow the user to:

- [X] Read, write and remove a file.
- [X] Create/remove folders.
- [X] Write a file as UTF-8 or UTF-8-bom easily.
- [X] Move/copy a file to another place.
- [X] Check if a path already exists.
- [X] Return error types.
- [X] Print in the terminal.
- [X] Convert images to `.dds` automatically.
- [ ] Resize images.

## Step 3: Basic HOI4 functions implementation
This gets tricky. If step 2 is completed, then by that point the base language is done, however the implementation of HOI4 functions won't be. Thus, the implementations will be stored at `libpdx.hpl`. You'll be able to import separate libraries like `libevent.hpl` if need be (like how you can with Win32). The reason why this can get tricky is that this might require the language's syntax to be changed. Each change can either be subtle and won't cause issues, or may require extensive rewrites of the code. Anyhow, for this step these are the functions that I strive to be implemented:
- [ ] Mod creation. (Note: Sort of done, however full functionality like tags aren't fully implemented and need more fine-tuning)
- [ ] Event creation.
- [ ] Nation creation.
- [ ] Proper localisation support for other languages.

## Step 4: Adding in additional basic programming features into HPL
Now that the language can build quite some HOI4 code, by now we should have a stable development environment. So now we can build-upon HPL and add new coding features. These features are, but not limited to:
- [ ] Arrays.
- [ ] Implement basic math (+, -, /, *) (Note: `++, --, +=, -=, *=, /=, %=` are supported).
- [X] Implement basic operators (&&, ||, ==, !=, >=, <=) (Note: only in `if` statements for now, and && isn't supported at the moment).
- [ ] Loops (`for (<variable> in <variable2>)`), (`for (<variable>, <condition>, <step>)`).

# Documentation
Since the language is still pretty expiremntal, all of the documentation for HPL are in source files and/or examples. The notes are there for anyone wanting to know why I do certain things like that for the sake of convenience, and it acts as a reminder for me sometimes.

## Syntax
The syntax and feature-set of HPL is almost identical to C's, features like structures, static types, declarations etc. are mostly the same.

However there are some additions as well as removals from C to make the language more approachable for HOI4 coders and tailored to HOI4 modding. The main changes are:
- There are no semicolons (`;`)
- There are HOI4-only types here (main one being `scope`)
- A variable can be declared dynamically or as a generic.

## Implemented features
A short list of things that are implemented with full functionality:
- Create and edit variables
- C-based structures (you can access and edit struct members)
- Declare functions with return types
- Execute a function and get its return
- Core functions
- Python's `f-string`
- Debug and logging modes
- Standard libraries for creating HOI4 mods (e.g. `libpdx.hpl`, `libcountry.hpl`)
- `if` statements
- Simple math (`++`, `--`, `+=`, `-=`, `=`, `/=`, `%=`)
- HOI4 scopes*

## Examples

[general](/examples/general/main.hpl) - Shows the general programming features of HPL and what you can do with it.

[country](/examples/country/main.hpl) - Creates a nation.

[event](/examples/event/main.hpl) - An example of creating a simple HOI4 event.

## Scope
Scopes aren't implemented at all. However, it is a very important type in HOI4 scripting and by default HPL, as the Paradox Wiki describes it, `Scopes select entities in order to check for triggers or apply effects.`.

Essentially, effects that you associate with HOI4 scripting (e.g. `add_stability`) can only be performed in scopes and nowhere else. However HOI4 scripting is the only language we know of that really uses scopes for results, as any other language would just have an `if` statement to check if the option got picked or not.

Scopes work pretty well in HOI4 scripting, however in HPL it's quite an issue for 2 reasons.

1. It makes it unclear when you can use modifiers in HPL code.
2. Since we're translating HPL scopes to HOI4 scripting scopes, it means that certain HPL features cannot make it into it when writing scopes. This only applies to features that don't have a HOI4 scripting equivalent/cannot be implemented by different ways.

To make things more clear for everyone involved using HPL, we've come up with 2 modes in HPL:

- Regular mode (non-scope mode)
- HOI4 scripting+ mode (scope mode)

In regular mode it's just HPL, meaning you can use the entire full feature-set of the language anywhere. In HOI4 scripting+ mode, you'll be essentially writing HOI4 scripting code with the available feature-set of HPL in scope mode. Backwards compatibility with regular HOI4 scripting would also be possible.

To come up with the best scope mode implementation, we've come up with 4 guidelines/required features that should be included in the implementation.

1. Syntax should be as simple and C-based as possible (no need to have overcomplicated syntax for something that's simple in HOI4 scripting).
2. Scope variables should exist for more options, portability, convenience for me to program in and backwards compatibility.
3. Being able to declare functions with scope required functionality. If such functionality is enabled, then the function will get a bonus variable with the scope variable.
4. Must be clear when the user is writing code in non-scope or scope mod.

### **Examples of how one of the scope modes could look**
**Function form**

```c
newOption(someEventVar, "da title") = {
	addStability("ROOT", 50) // If the AI/player picks this option, it gains 50% stability
}
```

**Variable form**
```c
scope savedCode = {
	addWarSupport("USA", -10) // Now we can use this scope variable anywhere we want.
}
```
**Using both examples in one**

```c
scope savedCode = {
	addStability("ROOT", 50)
	addWarSupport("USA", -10)
} // Now we can use this scope variable in any scope we want.

newOption(someEventVar, "da title 2") = {
	savedCode // This gets transformed into actual code when the interpreter reads and transforms it back to HPL and then finally HOI4 code.
}

```

# CLI options
**Note:** The most up to date list of CLI options is available via doing `hpl -help`.

```
ARGS:
        <FILE>                     Selected file to be interpreted.
OPTIONS:
        -help, -h                  Prints the available CLI options as well as the the version, authors, compiler and OS of the HPL executable.
        -debug, -g                 Enables all debug procedures (logging and printing debug information).
        -log, -l                   Logs and prints every noteworthy event that the interpreter has got.
        -strict, -s                Enables a strict mode, where you have a limited amount of available features to make less confusing code/massive mistakes (Barely implemented).
```

# Final notes
## Building HPL
If you're planning to build HPL, please note that the main programming environment isn't Windows, so expect possible errors and/or unusual behaviour on that platform, as from our experience it's much more buggy and annoying to program on Windows than it is on other platforms (due to mostly compiler implementations being weird and causing issues in code that works in one platform but doesn't in the other).

Here is my developer environment that I'll be using for most of the HPL work:

```
OS: macOS 12.6
Compiler: Apple clang version 14.0.0 (clang-1400.0.29.102)
Architecture: x86_64
C++ standard: c++17
```
In case I use Windows:
```
OS: Windows 10 (OS Build 19045.2251)
Compiler: Clang 15.0.3
Architecture: x86_64
C++ standard: c++20
```

## Cross-platform status
HPL should work perfectly on MacOS after each commit, as that is our main host system.

Compatibility on Windows should also be usually good, however it might not be in 100% of the cases (as Windows might do its own thing compared to MacOS/Linux).

Linux support is unofficial but should work 1 to 1 with the MacOS version, considering both are Unix.

## Dependencies
We usually do not like including pre-compiled dependencies in projects, as then it becomes a hassle to keep things cross-platform.

So for the majority of HPL, using dependecies is stricly forbidden and should only be used as a final resort. However, using dependencies in core functions is acceptable and won't cause any fuss (though again, must be a final resort if there aren't any header-only solutions or self-implementations the function).

The dependency can only be statically-compiled or header-only and must be cross-platform between all platforms that you can play HOI4 on (Windows, Mac and Linux).

## HPL Visual Studio Code extension
To make development with HPL much more colourful, fun and modern, programmer [Allyedge](https://github.com/Allyedge) has created a VSC extension for programming in HPL. For now it only has a syntax highlighter, however in the future it's gonna much more features to make development even easier (think of it like the C/C++ VSC extension).

To download it you can just look up 'HPL' in the marketplace, or go to [this link](https://marketplace.visualstudio.com/items?itemName=Allyedge.hpl).

## Credits
[SOIL2 (forked version)](https://github.com/EimaMei/SIL2) - for the `convertToDds` core function, HPL uses a modified version of SOIL2 to remove unneeded OpenGL requirements.

[Allyedge](https://github.com/Allyedge) - for creating the HPL VSC extension.
