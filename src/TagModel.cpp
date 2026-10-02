#include "TagModel.h"

#include "AttributeNames.h"

#include <Entry.h>
#include <Node.h>
#include <Query.h>
#include <String.h>
#include <Volume.h>
#include <VolumeRoster.h>

namespace {

void
read_string_attr(BNode& node, const char* attr, BString& out)
{
	out = "";
	char buf[2048];
	ssize_t n = node.ReadAttr(attr, B_STRING_TYPE, 0, buf, sizeof(buf) - 1);
	if (n > 0) {
		buf[n] = '\0';
		out = buf;
	}
}


void
split_csv(const BString& csv, BStringList& into)
{
	int32 start = 0;
	for (int32 i = 0; i <= csv.Length(); i++) {
		if (i == csv.Length() || csv[i] == ',') {
			BString tok;
			csv.CopyInto(tok, start, i - start);
			tok.Trim();
			if (tok.Length() > 0 && !into.HasString(tok))
				into.Add(tok);
			start = i + 1;
		}
	}
}


void
scan_volume(BVolume& volume, const char* predicate,
	const char* attr, bool splitCsv, BStringList& out)
{
	BQuery q;
	q.SetVolume(&volume);
	q.SetPredicate(predicate);
	if (q.Fetch() != B_OK)
		return;
	entry_ref ref;
	while (q.GetNextRef(&ref) == B_OK) {
		BNode node(&ref);
		if (node.InitCheck() != B_OK)
			continue;
		BString value;
		read_string_attr(node, attr, value);
		if (value.Length() == 0)
			continue;
		if (splitCsv) {
			split_csv(value, out);
		} else {
			value.Trim();
			if (value.Length() > 0 && !out.HasString(value))
				out.Add(value);
		}
	}
}

} // namespace


void
TagModel::Collect(Catalog& out)
{
	out.albums.MakeEmpty();
	out.tags.MakeEmpty();

	BVolumeRoster roster;
	BVolume volume;
	while (roster.GetNextVolume(&volume) == B_OK) {
		if (!volume.KnowsQuery())
			continue;
		scan_volume(volume,
			"(BEOS:TYPE==\"image/*\")&&(Photo:Album==\"*\")",
			SBX_ATTR_ALBUM, false, out.albums);
		scan_volume(volume,
			"(BEOS:TYPE==\"image/*\")&&(Photo:Tags==\"*\")",
			SBX_ATTR_TAGS, true, out.tags);
	}

	out.albums.Sort();
	out.tags.Sort();
}
