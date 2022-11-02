#include <libpdx.hcl>

string firstFile = "build/output/cringe.txt"
string secondFile
secondFile = "build/output/cringe2.txt"

createFolder("build/output")
createFile(firstFile)
writeFile(firstFile, "This is an utf-8 file, trolle!")
createFile("build/output/cringe2.txt", "Create an UTF-8 BOM immediately", true)

string txt = readFile(firstFile)
print(f"Contents of the first file: \"{txt}\"")

txt = readFile(secondFile)
print(f"Contents of the second file: \"{txt}\"")

print("Converting a jpg -> dds output: ", "")
print(convertToDds("examples/general/res/literal_evil.png", "build/output/literal_evil.dds"))