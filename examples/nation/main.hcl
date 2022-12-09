#include <libpdx.hcl>

newMod("build/output", "Test mod", "1.12.*")

country blr = newCountry("BLR", "White Ruthenia", {143, 143, 143}, 200)
// country ali = newCountry(tag = "HCL", "Alithron's nation", {200, 23, 210}, 200)
countrySetPolitics(blr, "neutrality", 25, 25, 25, 25)
//countrySetMisc(ali, -20, 40, 1)

print(blr)