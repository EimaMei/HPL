struct option {
	string title
	scope results
}

struct event {
	string namespace

	string title
	int id
	string description
	string picture
	bool isTriggeredOnly = true // By default 'isTriggeredOnly' is set to true
	bool isNewsEvent = false // 'isNewsEvent' is set to false by default
	option options[]
}

//event newEvent(string namespace, int id, string title, string description, string path = NULL) {
//	// some pseudo hc4 functions.
//	createFolder("build/events", false);
//	createFolder("build/localisation", false);
//	createFolder("build/gfx/event_pictures", false);
//	createFolder("build/interface", false);
//
//	fileBuffer1 = createFile("build/localisation/new_events_l_english.yml", "\xEF\xBB\xBFl_english:\n  ", std::ios::app);
//}
//
//addStability(string tag, int value)