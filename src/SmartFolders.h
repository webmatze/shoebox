#ifndef SHOEBOX_SMART_FOLDERS_H
#define SHOEBOX_SMART_FOLDERS_H

#include <Entry.h>
#include <ObjectList.h>
#include <String.h>
#include <SupportDefs.h>

namespace SmartFolders {

	struct Entry {
		BString   name;      // display label and filename
		BString   predicate; // BFS query predicate
		entry_ref file;      // backing file on disk
	};

	// Loads user-defined smart folders from
	// ~/config/settings/Shoebox/smartfolders/, sorted by name.
	void Load(BObjectList<Entry, true>& out);

	// Writes a new query file. Fails with B_FILE_EXISTS on duplicate name.
	status_t Create(const char* name, const char* predicate);

	// Renames (if changed) and rewrites the predicate attribute.
	status_t Update(const entry_ref& existing,
		const char* newName, const char* newPredicate);

	// Deletes the backing file.
	status_t Delete(const entry_ref& ref);

} // namespace SmartFolders

#endif
