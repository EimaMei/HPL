//#include <libpdx.hcl>
#include <libcountry.hcl>

newMod("build/output", "Test mod", "1.12.*")

country blr = newCountry(name = "White Ruthenia", tag = "BLR", color = {143, 143, 143}, capital = 200)

countrySetPolitics(country = blr, rulingParty = "neutrality", democratic = 25, fascist = 25, communist = 25, neutral = 25)
countrySetMisc(country = blr, stability = -20, warSupport = 40, researchSlots = 1)

print(blr)