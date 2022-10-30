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
- [X] Add returns to functions.
- [ ] Finalize how scopes work in general.

## Step 2: Core functions implementation
When the base code for the interpreter is done, it'll allow us to finally implement the core functions of the language to build HOI4 code. Without core functions, we won't be able to build HOI4 code. Some of these functions will allow the user to:

- [ ] Read and write a file.
- [ ] Create folders.
- [ ] Write a file as UTF-8 or UTF-8-bom easily.
- [ ] Get and set the current scope.
- [ ] Return error types.
- [X] Print in the terminal.
- [ ] Convert images to `.dds` automatically.
- [ ] Resize images.

## Step 3: Basic HOI4 functions implementation
This gets tricky. If step 2 is completed, then by that point the base language is done, however the implementation of HOI4 functions won't be. Thus, the implementations will be stored at `libpdx.hcl`. You'll be able to import separate libraries like `libevent.hcl` if need be (like how you can with Win32). The reason why this can get tricky is that this might require the language's syntax to be changed. Each change can either be subtle and won't cause issues, or may require extensive rewrites of the code. Anyhow, for this step these are the functions that I strive to be implemented:
- [ ] Mod creation.
- [ ] Nation creation.
- [ ] Event creation.
- [ ] Proper localisation.

## Step 4: Adding in basic programming features into HCL
Now that the language can build some basic hoi4 code, we now have a development environment. So now we can build-upon HCL and add new coding features. These features are, but not limited to:
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
Scopes aren't implemented at all. However, it is a **very** important type in HCL as it dictates when you can use HOI4-implemented functions in the code. This is due to how Paradox modding files work in general, where the results of a certain action are declared in a scope (for example, an option in an event would be a scope).

Due to this, it'll be required for me to think about how to implement scopes in HCL that'll:
1. Make it obvious when you can use HOI4 code without getting errors or just no output.
2. Not ruin/overcomplicate the syntax.

I have a few ideas on how I would implement such a type. One of them being that the user will be able to set if a function should set inside a scope or not. An example of it would be the [event example](examples/event/main.hcl), where inside the commented lines you can see `newEvent` have curly brackets and inside of it pseudo HOI4 code is being used. However for now it isn't required for me to implement scopes, as I still need to create the base language so as is right now, scopes won't be implemented for awhile.
