#include "Importer.h"

#include "AttributeNames.h"
#include "ExifReader.h"

#include <Directory.h>
#include <Entry.h>
#include <Mime.h>
#include <Node.h>
#include <Path.h>
#include <fs_attr.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

namespace {

void
process_file(const char* path, Importer::Result& r)
{
	r.scanned++;

	update_mime_info(path, 0, 1, B_UPDATE_MIME_INFO_NO_FORCE);

	BNode node(path);
	if (node.InitCheck() != B_OK) {
		r.errors++;
		return;
	}

	char type[B_MIME_TYPE_LENGTH];
	ssize_t s = node.ReadAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0,
		type, sizeof(type) - 1);
	if (s <= 0)
		return;
	type[s] = '\0';

	if (strncmp(type, "image/", 6) != 0)
		return;

	node.RemoveAttr("BEOS:TYPE");
	node.WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0,
		type, strlen(type) + 1);
	r.reindexed++;

	time_t capture = ExifReader::ReadCaptureTime(path);
	if (capture > 0) {
		int32 seconds = (int32)capture;
		node.WriteAttr(SBX_ATTR_CAPTURE_TIME, B_INT32_TYPE, 0,
			&seconds, sizeof(seconds));
	}
}


void
walk(BDirectory& dir, Importer::Result& r)
{
	BEntry entry;
	while (dir.GetNextEntry(&entry, false) == B_OK) {
		if (entry.IsDirectory()) {
			BDirectory sub(&entry);
			walk(sub, r);
			continue;
		}
		BPath path;
		if (entry.GetPath(&path) == B_OK)
			process_file(path.Path(), r);
	}
}

} // namespace


Importer::Result
Importer::ImportFolder(const entry_ref& folderRef)
{
	Result r = {0, 0, 0};
	BDirectory dir(&folderRef);
	if (dir.InitCheck() != B_OK)
		return r;
	walk(dir, r);
	fprintf(stderr, "[import] scanned=%d reindexed=%d errors=%d\n",
		(int)r.scanned, (int)r.reindexed, (int)r.errors);
	return r;
}
