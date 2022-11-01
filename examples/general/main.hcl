#include <libpdx.hcl>

string txt
createFolder("build/output")

createFile("build/output/cringe.txt")
writeFile("build/output/cringe.txt", "This is an utf-8 file, trolle!")
createFile("build/output/cringe2.txt", "Create an UTF-8 BOM immediately", true)

print("Contents of the first file: ", "")
txt = readFile("build/output/cringe.txt")
print(txt)
print("Contents of the second file: ", "")
print(readFile("build/output/cringe2.txt"))