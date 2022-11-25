#include "libmod.hcl"

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
	bool isTriggeredOnly
	bool isNewsEvent

	// Read-only
	bool init = false
	int options = 0
}


event newEvent(string namespace, int id, string title, string description, string imagePath) {
	string path = HCL_currentMod.path

	string locPath = f"{path}/localisation/hcl_events_l_english.yml"
	string eventPath = f"{path}/events/{namespace}.txt"
	string gfxName = replaceAll(replaceAll(getFilenameFromPath(imagePath), ".", "_"), "/", "_")
	string ddsName = replaceAll(replaceAll(getFilenameFromPath(imagePath), ".jpg", ".dds"), ".png", ".dds")

	createFolder(f"{path}/events")
	createFolder(f"{path}/localisation")
	createFolder(f"{path}/gfx/event_pictures")
	createFolder(f"{path}/interface")

	createFile(eventPath, f"add_namespace = {namespace}")
	createFile(locPath, "l_english:", true)
	createFile(f"{path}/interface/hcl_eventpictures.gfx", "spriteTypes = {")

	writeFile(eventPath, f"\n\n# \"{title}\" event\ncountry_event = {\n\tid = {namespace}.{id}\n\ttitle = {namespace}.{id}.t\n\tdesc = {namespace}.{id}.d\n\tpicture = GFX_{gfxName}\n\t\n\n}", "a")

	writeLocalisation(locPath, f"{namespace}.{id}.t", title)
	writeLocalisation(locPath, f"{namespace}.{id}.d", description)

	convertToDds(imagePath, f"{path}/gfx/event_pictures/{ddsName}")
	writeFile(f"{path}/interface/hcl_eventpictures.gfx", f"\n\nspriteType = {\n\tname = \"GFX_{gfxName}\"\n\ttexturefile = \"gfx/event_pictures/{ddsName}\"\n}", "a")

	return {namespace, id, title, description}
}


int newEventOption(event e, string title) {
	string path = HCL_currentMod.path

	string locPath = f"{path}/localisation/hcl_events_l_english.yml"
	string eventPath = f"{path}/events/{e.namespace}.txt"

	writeToLine(eventPath, -2, f"\n\toption = {\n\t\tname = {e.namespace}.{e.id}.1\n\t}\n")
	writeLocalisation(locPath, f"{e.namespace}.{e.id}.1", title)
	//e.options++
}