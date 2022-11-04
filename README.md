# What is HCL?
The HOI4 Coding Language (HCL) is a high-level interpreter programming language for HOI4 that is set to make HOI4 scripting much better, while also adding a bunch of new features to make HOI4 programming enjoyable.

# Why HCL?
Because the current HOI4 scripting "language" lacks very basic programming features (which makes the code more ugly and longer), overcomplicates things for the sake of simplicity, always breaks when a new DLC launches and is just generally not fun to program in. This is why I always refer to the current HOI4 coding as "scripting", as its very basic and lacks the many features that programmer like myself take for granted.

The solution to this? Create an actual programming language designed for HOI4 coders. HCL will attempt to fix all of these issues by doing the following:
- Implement basic language features that many modern languages have (structures, loops etc).
- Rework the syntax of the HOI4 scripting language to make it more C-based.
- Make the elements of HOI4 modding easier to implement (automatically converting images to `.dds`, resizing flags to the correct size and format, make `if` statements and variables behave like how they do in proper languages, and so on)
- Have proper error checking when building code so that mods don't have thousands of errors/crash at startup.
- Enforcing stability so that code doesn't break each time a new DLC releases.

# Current status of HCL
Currently HCL is ***very*** expiremental, as such bugs should be expected and reported. However it now already has quite a few basic language features to toy around with, as well as the ability to use `libpdx.hcl` to ouput a few pieces of HOI4 code. With enough time (months, maybe a year or two?) it'll be stable enough for normal use cases for mods (hell, maybe even for full-scale productions but I don't expect that anytime soon, if at all).
# Current roadmap
## Step 1: Building the base code
Currently the language is barely done and it'll take awhile before any random modder will be able to use it without encountering some bug/unimplemented HOI4 functions. As such, these basic features must be implemented:

- [X] Save variables in memory.
- [X] Being able to edit variables at runtime.
- [X] Finish basic structures.
- [X] Access a struct's member and also edit it.
- [X] Save a function (and its arguments) in memory, while also retaining its code.
- [X] Delete variables when they're out of scope.
- [X] Execute functions when they're used.
- [X] Add returns to core typed functions.
- [ ] Add returns to non-core typed functions.
- [X] Being able to set the value of a __CORE TYPED__ variable from a function's return.
- [ ] Being able to set the value of a __STRUCT TYPED__ variable from a function's return (ties in with "Add returns to non-core typed functions").
- [X] Add f-string.
- [ ] Add more runtime errors (defining variables that already exist, too many curly brackets)
- [ ] Make error/warnings reports more pretty (more aline with how GCC does it).
- [ ] Multiline variables
- [ ] Finalize how scopes work in general.

## Step 2: Core functions implementation
When the base code for the interpreter is done, it'll allow us to finally implement the core functions of the language. Without core functions, we won't be able to build HOI4 code. Some of these functions will allow the user to:

- [X] Read, write and remove a file.
- [X] Create/remove folders.
- [X] Write a file as UTF-8 or UTF-8-bom easily.
- [X] Move/copy a file to another place.
- [X] Check if a path already exists.
- [ ] Get and set the current scope.
- [X] Return error types.
- [X] Print in the terminal.
- [X] Convert images to `.dds` automatically.
- [ ] Resize images.

## Step 3: Basic HOI4 functions implementation
This gets tricky. If step 2 is completed, then by that point the base language is done, however the implementation of HOI4 functions won't be. Thus, the implementations will be stored at `libpdx.hcl`. You'll be able to import separate libraries like `libevent.hcl` if need be (like how you can with Win32). The reason why this can get tricky is that this might require the language's syntax to be changed. Each change can either be subtle and won't cause issues, or may require extensive rewrites of the code. Anyhow, for this step these are the functions that I strive to be implemented:
- [ ] Mod creation.
- [ ] Nation creation.
- [ ] Event creation.
- [ ] Proper localisation.

## Step 4: Adding in additional basic programming features into HCL
Now that the language can build quite some HOI4 code, by now we should have a stable development environment. So now we can build-upon HCL and add new coding features. These features are, but not limited to:
- [ ] Arrays.
- [ ] Implement basic math (+, -, /, *).
- [ ] Implement basic operators (&&, ||, ==, !=, >=, <=).
- [ ] Loops (`for (<variable> in <variable2>)`), (`for (<variable>, <condition>, <step>)`).

# Documentation
Since the language is still pretty expiremntal, all of the documentation for HCL are in source files and/or examples. The notes are there for anyone wanting to know why I do certain things like that for the sake of convenience, and it acts as a reminder for me sometimes.

# Syntax
The syntax and feature-set of HCL is almost identical to C's, features like structures, static types, declarations etc. are mostly the same. However there are some additions/removals from C to make the language more approachable for HOI4 coders and tailored to HOI4modding. The main changes are:
- There are no semicolons (`;`).
- There are HOI4-only types here (main one being 'scope').
- A variable can be declared dynamically or as a generic.

# Implemented features
## Variables
To declare a variable, you must provide the type, name and optionally the value (`<type> <name> = [value]`). You can also do `name := <value>`, which'll automatically get the type, or just declare a generic type `var num = 3`. Editing a variable is the same like everywhere else.
## Structures
Structures are defined and work like in C. You can define a structure with default options and that will carry on to the variable that you'll define.
```c
struct info {
	string desc = "Something mildly interesting here"
	int value = 50
}

info var // This variable is saved as `info var = {"Something mildly interesting here", 50}`

...

info var = {"Something new here"} // While this would be saved as {"Something new here", 50}
```
However, as of now, you *cannot* init variables with out of order arguments (eg. `{.value = 25, .desc = "Out of order shenanigans!"}`)
## Scope
Scopes aren't implemented at all. However, it is a **very** important type in HCL as it dictates when you can use quite a lot of the HOI4-implemented functions in the code. This is due to how Paradox modding files work in general, where the results of a certain action are declared in a scope to make sense (for example, an option in an event would be a scope). As such, you cannot just declare `addStability("SOV", 20)` randomly in the code as neither HCL nor HOI4 would understand where, why or when that action would happen in runtime.

Due to this, it'll be required for me to think about how to implement scopes in HCL that'll:
1. Make it obvious when you can use HOI4 code without getting errors or just no output.
2. Not ruin/overcomplicate the syntax.
3. Fit in nicely with the C-ish syntax.

I have a few ideas on how I would implement such a type. One of them being that a scope variable would be declared in curly brackets, and inside those brackets you can save HOI4 code (possible even having cross-compatibility with old HOI4 code).

### **Examples of how it could look**
1. **Funcion form**
```c
newOption(someEventVar, "da title", "description whatever") = {
	addStability("ROOT", 50) // If the AI/player picks this option, it gains 50% stability
}
```
2. **Variable form**
```c
scope savedCode = {
	addWarSupport("USA", -10) // Now I can use this scope variable anywhere.
}
```
3. **Using both examples as one**
```c
scope savedCode = {
	addStability("ROOT", 50)
	addWarSupport("USA", -10)
} // Now I can use this scope variable in any scope I want.

newOption(someEventVar, "da title 2", "another description whatever") = {
	savedCode // This gets transformed into actual code when the interpreter reads and transforms it back to HCL and then finally HOI4 code.
}

```

However for now it isn't required for me to implement scopes, as I still need to create the base language so as is right now, scopes won't be implemented for awhile.
# Final notes
## Building HCL
If you're planning to build HCL, please note that my main programming environment isn't Windows, so expect possible errors and/or unsual behaviours on that platform, as from my experience it's much more buggy and annoying to program on Windows than it is on other platforms (due to mostly compiler implementations being whack and causing issues in code that works in one platform but doesn't in the other). Here is my developer environment that'll be using for most of the HCL work:
```
OS: macOS 12.6
Compiler: Apple clang version 14.0.0 (clang-1400.0.29.102)
Architecture: x86_64
C++ standard: c++17
```
In case I use Windows:
```
OS: Windows 10
Compiler: Clang 15.0.3
Architecture: x86_64
C++ standard: c++17
```
## Cross-platform status
HCL should work perfectly on MacOS after each commit, as that is my main host system. Compatibility on Windows should also be usually good, however it might not be 100% the case. As for Linux, I have no clue. It probably works on Linux too, but I don't know as I haven't tested it out myself yet and I don't plan to add official support to it until HCL will be more completed.
## Dependencies
I usually do not like including pre-compiled dependencies in projects, as then it becomes a hassle to keep things cross-platform. So for the majority of HCL, using dependecies is stricly forbidden and should be used as a final resort. However, using dependencies in core functions is acceptable and won't cause any fuss (though again, must be a final resort if there aren't any header-only solutions/self-implementations the function). The dependency can only be statically-compiled/header-only and must be cross-platform between all platforms that you can use HOI4 on (Windows, Mac and Linux).
## Credits
[SOIL2 (forked version)](https://github.com/EimaMei/SIL2) - for the `convertToDds` core function, HCL uses a modified version of SOIL2 to remove unneeded OpenGL requirements.
