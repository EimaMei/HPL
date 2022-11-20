#include <libpdx.hcl>

//string desc = "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest."
// Later on, multiple languages will be supported. An example of how it would look:
// string desc = {
//     "english" : "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest.",
//     "lithuanian" : "Pakartotiniai sukilimai Austrijoje aiškiai rodo, kad tenykščiai žmonės trokšta vienybės su mūsų Reichu. Dabar atėjo laikas imtis veiksmų ir leisti pasireikšti žmonių valiai.",
//     "german" : "<smth in german, too lazy to use a translator>"
// }
//removeFolder("build/output")


removeFolder("build/output")
createFolder("build/output")

newMod("Test mod", "1.12.*")

event v = newEvent("hcl_events", 56, "nah broooo", "This ain't fr!!!!!!!!!!!!!!!", "examples/event/res/literal_evil.png")
print(v)
newEventOption(v, "Damng!")