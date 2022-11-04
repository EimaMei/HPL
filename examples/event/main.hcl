#include <libpdx.hcl>

// Later on, multiple languages will be supported. An example of how it would look:
// string desc = {
//     "english" : "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest.",
//     "lithuanian" : "Pakartotiniai sukilimai Austrijoje aiškiai rodo, kad tenykščiai žmonės trokšta vienybės su mūsų Reichu. Dabar atėjo laikas imtis veiksmų ir leisti pasireikšti žmonių valiai.",
//     "german" : "<smth in german, too lazy to use a translator>"
// }
//removeFolder("build/output")

string desc = "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest."

removeFolder("build/output")
createFolder("build/output")
newEvent("bro", 2, "nah broooo", "This ain't fr!!!!!!!!!!!!!!!", "res/remtard.jpg")