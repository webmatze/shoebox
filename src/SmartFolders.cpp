#include "SmartFolders.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Node.h>
#include <Path.h>

#include <errno.h>
#include <string.h>

namespace {

const char* kQueryMime     = "application/x-vnd.Be-query";
const char* kPredicateAttr = "_trk/qrystr";
const char* kSubdir        = "Shoebox/smartfolders";

status_t
settings_dir(BPath& out, bool create)
{
	status_t s = find_directory(B_USER_SETTINGS_DIRECTORY, &out, create);
	if (s != B_OK)
		return s;
	s = out.Append(kSubdir);
	if (s != B_OK)
		return s;
	if (create)
		create_directory(out.Path(), 0755);
	return B_OK;
}


int
compare_entries(const SmartFolders::Entry* a, const SmartFolders::Entry* b)
{
	return a->name.ICompare(b->name);
}

} // namespace


void
SmartFolders::Load(BObjectList<Entry, true>& out)
{
	out.MakeEmpty();

	BPath dir;
	if (settings_dir(dir, false) != B_OK)
		return;

	BDirectory bdir(dir.Path());
	if (bdir.InitCheck() != B_OK)
		return;

	BEntry entry;
	while (bdir.GetNextEntry(&entry, false) == B_OK) {
		if (!entry.IsFile())
			continue;

		char name[B_FILE_NAME_LENGTH];
		if (entry.GetName(name) != B_OK)
			continue;

		entry_ref ref;
		if (entry.GetRef(&ref) != B_OK)
			continue;

		BNode node(&entry);
		if (node.InitCheck() != B_OK)
			continue;

		char buf[2048];
		ssize_t n = node.ReadAttr(kPredicateAttr, B_STRING_TYPE, 0,
			buf, sizeof(buf) - 1);
		if (n <= 0)
			continue;
		buf[n] = '\0';

		Entry* e = new Entry();
		e->name = name;
		e->predicate = buf;
		e->file = ref;
		out.AddItem(e);
	}

	out.SortItems(compare_entries);
}


status_t
SmartFolders::Create(const char* name, const char* predicate)
{
	BPath dir;
	status_t s = settings_dir(dir, true);
	if (s != B_OK)
		return s;

	BPath filePath(dir.Path(), name);
	BFile file(filePath.Path(),
		B_READ_WRITE | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (file.InitCheck() != B_OK)
		return file.InitCheck();

	file.WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0,
		kQueryMime, strlen(kQueryMime) + 1);
	file.WriteAttr(kPredicateAttr, B_STRING_TYPE, 0,
		predicate, strlen(predicate) + 1);
	return B_OK;
}


status_t
SmartFolders::Update(const entry_ref& existing,
	const char* newName, const char* newPredicate)
{
	BEntry entry(&existing);
	if (entry.InitCheck() != B_OK)
		return entry.InitCheck();

	char currentName[B_FILE_NAME_LENGTH];
	entry.GetName(currentName);

	if (strcmp(currentName, newName) != 0) {
		status_t s = entry.Rename(newName, false);
		if (s != B_OK)
			return s;
	}

	BNode node(&entry);
	if (node.InitCheck() != B_OK)
		return node.InitCheck();

	node.RemoveAttr(kPredicateAttr);
	return node.WriteAttr(kPredicateAttr, B_STRING_TYPE, 0,
		newPredicate, strlen(newPredicate) + 1) >= 0 ? B_OK : errno;
}


status_t
SmartFolders::Delete(const entry_ref& ref)
{
	BEntry entry(&ref);
	if (entry.InitCheck() != B_OK)
		return entry.InitCheck();
	return entry.Remove();
}
