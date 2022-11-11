#read once

struct modTags {
    bool gameplay = false
    bool historical = false
}

string HCL_currentModName

int newMod(string name, string supportedVersion) {
    createFolder(f"build/output/{name}")

    createFile(f"build/output/{name}.mod", f"name = \"{name}\"\nsupported_version = \"{supportedVersion}\"\ntag = {\n\t\"Gameplay\"\n\t\"Historical\"\n}\npath = \"mod/{name}\"")
    createFile(f"build/output/{name}/descriptor.mod", f"name = \"{name}\"\nsupported_version = \"{supportedVersion}\"\ntag = {\n\t\"Gameplay\"\n\t\"Historical\"\n}")
    HCL_currentModName = name
}