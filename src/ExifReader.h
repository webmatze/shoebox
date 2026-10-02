#ifndef SHOEBOX_EXIF_READER_H
#define SHOEBOX_EXIF_READER_H

#include <time.h>

namespace ExifReader {

	// Parses the JPEG's EXIF APP1 segment for DateTimeOriginal (0x9003),
	// falling back to the main IFD's DateTime (0x0132). Returns epoch
	// seconds, or 0 if the file isn't JPEG, has no EXIF, or no datetime
	// tag was found. Treats EXIF datetimes as local time (per spec).
	time_t ReadCaptureTime(const char* path);

} // namespace ExifReader

#endif
