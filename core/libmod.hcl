#read once

struct modTags {
    bool gameplay = false
    bool historical = false
}

struct mod {
    string name
    string supportedVersion
    modTags tags
    string path
}

mod HCL_currentMod

int newMod(string path, string name, string supportedVersion)
{
    HCL_currentMod = {name, supportedVersion, {}, f"{path}/{name}"}
    removeFolder(path)
    createFolder(HCL_currentMod.path)


    createFile(f"{path}/{name}.mod", f"name = \"{name}\"\nsupported_version = \"{supportedVersion}\"\ntag = {\n\t\"Gameplay\"\n\t\"Historical\"\n}\npath = \"mod/{name}\" ")
    createFile(f"{path}/{name}/descriptor.mod", f"name = \"{name}\"\nsupported_version = \"{supportedVersion}\"\ntag = {\n\t\"Gameplay\"\n\t\"Historical\"\n}")


    return 0
}