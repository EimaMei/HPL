#include <libpdx.hcl>

removeFolder("build/output")
createFolder("build/output")

newMod("Test mod", "1.12.*")

country ukraine = newCountry("UKR", "Ukraine", {255, 0, 0}, 200)