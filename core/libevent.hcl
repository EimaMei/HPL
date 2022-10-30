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
	//option options[]
}

event newEvent(string namespace, int id, string title, string description, string path = "something") {
	print(namespace)
	print(id)
	print(title)
	print(description)
	print(path)
}