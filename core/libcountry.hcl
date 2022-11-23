#include "libmod.hcl"

struct country {
    string tag
    string name
    int capital
}

struct rgb {
    int r // int r, g, b
    int g
    int b
}

country newCountry(string tag, string name, rgb color, int capital) {
    string path = f"build/output/{HCL_currentModName}"

    createFolder(f"build/output/{HCL_currentModName}/common/country_tags")
	createFolder(f"{path}/common/countries")
	createFolder(f"{path}/common/characters")

    createFile(f"{path}/common/countries/{tag}.txt", "graphical_culture = commonwealth_gfx\ngraphical_culture_2d = commonwealth_2d")
    createFile(f"{path}/common/countries/colors.txt", f"{tag} = {\n\trgb { {color.r} {color.g} {color.b} }\n\tcolor_ui { {color.r} {color.g} {color.b} }\n}\n\n")

    return {tag, name, capital}
}

// setFlag