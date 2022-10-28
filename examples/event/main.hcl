	#include <libpdx.hcl> // Same as doing #include "../../standard/libpdx.hcl"

	struct info {
		string desc = "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest."
		int value = -5
	}
	// Later on, multiple languages will be supported. An example of how it would look:
	// string desc = {
	//     "english" : "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest.",
	//     "lithuanian" : "Pakartotiniai sukilimai Austrijoje aiškiai rodo, kad tenykščiai žmonės trokšta vienybės su mūsų Reichu. Dabar atėjo laikas imtis veiksmų ir leisti pasireikšti žmonių valiai.",
	//     "german" : "insert funny german jokes here"
	// }

	info var = {"Wait a minute, I've seen this one before!"}
	string anotherdescriptionyayayayay = "Repeated uprisings in Austria clearly show that the people there long to be united with our Reich. It is now time to take action and allow the will of the people to be made manifest."
	bool cring = false
	anotherdescriptionyayayayay = "test"
	//var.desc = "A different description, huh?"

	//event anschluss = newEvent("germany", 2, "Anschluss", var.desc, "res/image.png") { // The Main scope (event).
	//	.options[0] = { // The sub-scope (anschluss.options[0])
	//		.title = "How can anyone say that Austria is not German?!"
	//		
	//		.results = { // Technically the sub-sub-scope (here we can actually have hoi4 code here)
	//			addStability("ROOT", var.value)
	//		}  
	//	} //The subscope ends here
	//
	//	.options[1] = { // Another sub-scope starts (anschluss.options[1])
	//		.title = "nah"
	//		
	//		.results = {
	//			var.value = 20
	//			addStability("USA", val) // This should be 20, not -5!
	//		}  
	//	}
	//}