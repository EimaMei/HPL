#read once

struct modTags {
    bool gameplay = false
    bool historical = false
}

string HCL_currentModName

int newMod(string name, string supportedVersion) {
    createFile(f"build/output/{name}.mod", f"name = \"{name}\"\nsupported_version = \"{supportedVersion}\"\ntag = {")
    createFolder(f"build/output/{name}")
    writeFile(f"build/output/{name}.mod", f"\n\t\"Gameplay\"\n\t\"Historical\"\n}\npath = \"mod/{name}\"", "a")
    HCL_currentModName = name
}