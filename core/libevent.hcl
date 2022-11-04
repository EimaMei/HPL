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

event newEvent(string namespace, int id, string title, string description, string path) {
	string locPath = "build/output/localisation/hcl_events_l_english.yml"
	string eventPath = f"build/output/events/{namespace}.txt"
	string gfxName = replaceAll(replaceAll(getFilenameFromPath(path), ".", "_"), "/", "_")
	string ddsName = replaceAll(replaceAll(getFilenameFromPath(path), ".jpg", ".dds"), ".png", ".dds")

	createFolder("build/output/events")
	createFolder("build/output/localisation")
	createFolder("build/output/gfx/event_pictures")
	createFolder("build/output/interface")

	createFile(eventPath, f"add_namespace = {namespace}")
	createFile(locPath, "l_english:", true)
	createFile("build/output/interface/hcl_eventpictures.gfx", "spriteTypes = {")

	writeFile(eventPath, f"\n\n# \"{title}\" event\ncountry_event = {\n\tid = {namespace}.{id}\n\ttitle = {namespace}.{id}.t\n\tdesc = {namespace}.{id}.d\n\tpicture = GFX_{gfxName}", "a")
	writeFile(eventPath, f"\n}", "a")

	writeLocalisation(locPath, f"{namespace}.{id}.t", title)
	writeLocalisation(locPath, f"{namespace}.{id}.d", description)

	convertToDds(path, f"build/output/gfx/event_pictures/{ddsName}")
	writeFile("build/output/interface/hcl_eventpictures.gfx", f"\n\nspriteType = {\n\tname = \"GFX_{gfxName}\"\n\ttexturefile = \"gfx/event_pictures/{ddsName}\"\n\t\n}", "a")
	//return {names, id, title, description, path}
}