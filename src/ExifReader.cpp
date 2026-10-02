#include "ExifReader.h"

#include <File.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace {

uint16_t
read_u16(const uint8_t* p, bool little)
{
	return little ? (uint16_t)(p[0] | (p[1] << 8))
	              : (uint16_t)((p[0] << 8) | p[1]);
}


uint32_t
read_u32(const uint8_t* p, bool little)
{
	return little
		? (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24))
		: (uint32_t)((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}


time_t
parse_datetime(const char* s)
{
	struct tm t;
	memset(&t, 0, sizeof(t));
	if (sscanf(s, "%d:%d:%d %d:%d:%d",
			&t.tm_year, &t.tm_mon, &t.tm_mday,
			&t.tm_hour, &t.tm_min, &t.tm_sec) != 6) {
		return 0;
	}
	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_isdst = -1;
	return mktime(&t);
}


// Scans an IFD at tiff[ifdOffset..] for an ASCII tag with the given id,
// copies the (up to 19 char) value into outDate and NUL-terminates.
bool
read_ifd_ascii(const uint8_t* tiff, size_t size, uint32_t ifdOffset,
	uint16_t targetTag, bool little, char outDate[20])
{
	if (ifdOffset + 2 > size)
		return false;
	uint16_t numEntries = read_u16(tiff + ifdOffset, little);
	uint32_t base = ifdOffset + 2;
	if (base + (uint32_t)numEntries * 12 > size)
		return false;

	for (uint16_t i = 0; i < numEntries; i++) {
		const uint8_t* entry = tiff + base + i * 12;
		uint16_t tag = read_u16(entry, little);
		uint16_t type = read_u16(entry + 2, little);
		uint32_t count = read_u32(entry + 4, little);

		if (tag != targetTag || type != 2 /* ASCII */ || count != 20)
			continue;

		uint32_t off = read_u32(entry + 8, little);
		if (off + 20 > size)
			return false;
		memcpy(outDate, tiff + off, 19);
		outDate[19] = '\0';
		return true;
	}
	return false;
}


uint32_t
find_exif_ifd(const uint8_t* tiff, size_t size, uint32_t ifd0, bool little)
{
	if (ifd0 + 2 > size)
		return 0;
	uint16_t numEntries = read_u16(tiff + ifd0, little);
	uint32_t base = ifd0 + 2;
	if (base + (uint32_t)numEntries * 12 > size)
		return 0;

	for (uint16_t i = 0; i < numEntries; i++) {
		const uint8_t* entry = tiff + base + i * 12;
		if (read_u16(entry, little) == 0x8769 /* ExifIFDPointer */)
			return read_u32(entry + 8, little);
	}
	return 0;
}


time_t
parse_tiff_block(const uint8_t* tiff, size_t size)
{
	if (size < 8)
		return 0;

	bool little;
	if (tiff[0] == 'I' && tiff[1] == 'I')
		little = true;
	else if (tiff[0] == 'M' && tiff[1] == 'M')
		little = false;
	else
		return 0;

	if (read_u16(tiff + 2, little) != 0x002A)
		return 0;

	uint32_t ifd0 = read_u32(tiff + 4, little);

	char date[20];

	// Preferred: ExifIFD / DateTimeOriginal (0x9003)
	uint32_t exifIfd = find_exif_ifd(tiff, size, ifd0, little);
	if (exifIfd != 0
		&& read_ifd_ascii(tiff, size, exifIfd, 0x9003, little, date)) {
		return parse_datetime(date);
	}

	// Fallback: main IFD / DateTime (0x0132)
	if (read_ifd_ascii(tiff, size, ifd0, 0x0132, little, date))
		return parse_datetime(date);

	return 0;
}

} // namespace


time_t
ExifReader::ReadCaptureTime(const char* path)
{
	BFile file(path, B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return 0;

	uint8_t marker[2];
	if (file.Read(marker, 2) != 2)
		return 0;
	if (marker[0] != 0xFF || marker[1] != 0xD8)
		return 0;

	while (true) {
		if (file.Read(marker, 2) != 2)
			return 0;
		if (marker[0] != 0xFF)
			return 0;

		uint8_t type = marker[1];
		if (type == 0xD9 /* EOI */ || type == 0xDA /* SOS */)
			return 0;

		uint8_t lenBuf[2];
		if (file.Read(lenBuf, 2) != 2)
			return 0;
		uint16_t segLen = (uint16_t)((lenBuf[0] << 8) | lenBuf[1]);
		if (segLen < 2)
			return 0;
		uint16_t payloadLen = segLen - 2;

		if (type == 0xE1 /* APP1 */) {
			uint8_t* payload = new uint8_t[payloadLen];
			if (file.Read(payload, payloadLen) != payloadLen) {
				delete[] payload;
				return 0;
			}
			if (payloadLen > 6
				&& memcmp(payload, "Exif\0\0", 6) == 0) {
				time_t t = parse_tiff_block(payload + 6, payloadLen - 6);
				delete[] payload;
				return t;
			}
			delete[] payload;
		} else {
			file.Seek(payloadLen, SEEK_CUR);
		}
	}
}
