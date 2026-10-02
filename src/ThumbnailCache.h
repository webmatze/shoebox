#ifndef SHOEBOX_THUMBNAIL_CACHE_H
#define SHOEBOX_THUMBNAIL_CACHE_H

#include <Entry.h>
#include <SupportDefs.h>

class BBitmap;

namespace ThumbnailCache {

	// Max dimension of a generated thumbnail (preserves aspect ratio).
	const int32 kThumbSize = 256;

	// Returns a caller-owned BBitmap, or nullptr on failure. Uses the
	// Shoebox:Thumb256 attribute as a persistent cache: reads it if present,
	// otherwise decodes the source image, scales, and writes the result back.
	BBitmap* LoadOrGenerate(const entry_ref& ref);

} // namespace ThumbnailCache

#endif
