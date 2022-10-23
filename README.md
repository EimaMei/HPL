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

- Save variables in memory.
- Being able to edit variables at runtime.
- Access the struct's member and also edit it.
- Save the function (and its arguments) in memory, while also retaining the code.
- Execute functions when they're used.
- Finalize how scopes work in general.
- Many, many other things that I currently can't think of.
## Step 2: Core functions implementation
When the base code for the intepreter is done, it'll allow us to finally implement the core functions of the language to build HOI4 code. Without core functions, we won't be able to build HOI4 code. Some of these functions will allow the user to:
- Read and write a file.
- Create folders.
- Write a file as UTF-8 or UTF-8-bom easily.
- Get and set the current scope.
- Return error types.
- Print in the terminal (format: `<DATE>:<file>: <message>\n`)
- Convert images to `.dds`.
- Resize images.
## Step 3: Basic HOI4 functions implementation
This gets tricky. If step 2 is completed, then by that point the base language is done, however the implementation of HOI4 functions won't be. Thus, the implementations will be stored at `libpdx.hcl`. You'll be able to import seperate libraries like `libevent.hcl` if need be (like how you can with Win32). The reason why this can get tricky is that this might require the language's syntax to be changed. Each change can either be subtle and won't cause issues, or may require extensive rewrites of the code. Anyhow, for this step these are the functions that I strive to be implemented:
- Mod creation.
- Nation creation.
- Event creation.
- Proper localisation.
## Step 4: Adding in basic programming features into HCL
Now that the language can build some basic hoi4 code, we now have a development environment. So now we can build-upon HCL and add new coding features. These features are, but not limited to:
- Arrays.
- Implement basic math (+, -, /, *).
- Implement operators (&&, ||, ==, !=, >=, <=).
- Loops (`for (<variable> in <variable2>)`) and (`for (i in range(<num>, [num2], [step]))`).
# Syntax
The syntax and feature-set of HCL is almost identical to C's, features like structures, static types, declarations etc. are the same. However there are some additions/removals from C to make the language for approachable for new learners, such as:
- There are no semicolons (`;`).
- A `main` entry isn't needed.
- There will be only a few main types for the user to use.
- Declare a variable with a dynamic type; 
# Implemented features
## Variables
To declare a variable, you must provide the type, name and optionally the value (`<type> <name> = [value]`). You can also do `name := <value>`, which'll automatically get the type.
## Structures
Structures are defined like in C:
```c
struct <name> {
	// variables go here
}
```
However you must know that each struct is also treated like a subscope. This doesn't mean anything for now, but in the future you'll be able to get/set information of the scope when required.
## Scope
### Scopes in functions
N/A for the moment
### Scopes in structs
> However you must know that each struct is also treated like a subscope. This doesn't mean anything for now, but in the future you'll be able to get/set information of the scope when required.
