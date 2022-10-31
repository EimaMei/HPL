struct option {
	string title
	scope results
}

struct event {
	string namespace

	int id
	string title
	string description
	string picture
	bool isTriggeredOnly = true // By default 'isTriggeredOnly' is set to true
	bool isNewsEvent = false // 'isNewsEvent' is set to false by default
	//option options[]
}

event newEvent(string namespace, int id, string title, string description, string path = "something") {
	createFolder("build/output/events")
	createFolder("build/output/localisation")
	createFolder("build/output/event_pictures")
	createFolder("build/output/interface")

	return {names, id, title, description, path}
}