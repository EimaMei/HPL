#include <libpdx.hcl>

newMod("build/output", "Test mod", "1.12.*")

country ali = newCountry("HCL", "Alithron's nation", {200, 23, 210}, 200)
countrySetPolitics(ali, "fascist", 10, 20, 65, 5)
countrySetMisc(ali, -20, 40, 1)