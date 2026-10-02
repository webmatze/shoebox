#ifndef SHOEBOX_TAG_MODEL_H
#define SHOEBOX_TAG_MODEL_H

#include <StringList.h>

namespace TagModel {

	struct Catalog {
		BStringList albums; // distinct Photo:Album values
		BStringList tags;   // distinct comma-split Photo:Tags values
	};

	// Scans every query-capable volume, reads Photo:Album / Photo:Tags
	// attributes, dedupes and sorts. Synchronous; cheap for small libraries.
	void Collect(Catalog& out);

} // namespace TagModel

#endif
