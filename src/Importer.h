#ifndef SHOEBOX_IMPORTER_H
#define SHOEBOX_IMPORTER_H

#include <Entry.h>
#include <SupportDefs.h>

namespace Importer {

	struct Result {
		int32 scanned;    // files visited
		int32 reindexed;  // image files whose BEOS:TYPE was rewritten
		int32 errors;     // files we couldn't open
	};

	// Walks the directory recursively. For each regular file: makes sure
	// BEOS:TYPE is set via update_mime_info, and if the type is image/*,
	// forces a rewrite of BEOS:TYPE so BFS inserts the file into the index.
	Result ImportFolder(const entry_ref& folderRef);

} // namespace Importer

#endif
