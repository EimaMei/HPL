#include "libmod.hcl"

struct country {
    string tag
    string name
    int capital
    string oob
}

struct rgb {
    int r // int r, g, b
    int g
    int b
}

country newCountry(string tag, string name, rgb color, int capital, string oob = "") {
    string path = HCL_currentMod.path

    createFolder(f"{path}/common/country_tags")
	createFolder(f"{path}/common/countries")
    createFolder(f"{path}/history/countries")

    createFile(f"{path}/common/countries/{tag}.txt", f"graphical_culture = commonwealth_gfx\ngraphical_culture_2d = commonwealth_2d\n\n{tag} = {\n\trgb { {color.r} {color.g} {color.b} }\n\tcolor_ui { {color.r} {color.g} {color.b} }\n}")
    createFile(f"{path}/history/countries/{tag} - {name}.txt", f"capital = {capital}\noob = \"{oob}\"\n\nset_research_slots = 3\nset_stability = 0.5\nset_war_support = 0.5\n\nset_popularities = {\n\tdemocratic = 25\n\tcommunism = 25\n\tfascism = 25\n\tneutrality = 25\n}")

    return {tag, name, capital, oob}
}

int countrySetPolitics(string rulingParty, int democratic, int communist, int fascist, int neutral) {
    //writeToFile("")
}

// setFlag