#ifndef SHOEBOX_ATTRIBUTE_NAMES_H
#define SHOEBOX_ATTRIBUTE_NAMES_H

// EXIF-derived (written by ingest; indexed for queries)
#define SBX_ATTR_CAPTURE_TIME "Media:CaptureTime" // B_INT32_TYPE (epoch seconds)
#define SBX_ATTR_CAMERA       "Media:Camera"      // B_STRING_TYPE "Make Model"
#define SBX_ATTR_LENS         "Media:Lens"        // B_STRING_TYPE
#define SBX_ATTR_WIDTH        "Media:Width"       // B_INT32_TYPE
#define SBX_ATTR_HEIGHT       "Media:Height"      // B_INT32_TYPE
#define SBX_ATTR_ISO          "Media:ISO"         // B_INT32_TYPE
#define SBX_ATTR_FOCAL_LENGTH "Media:FocalLength" // B_FLOAT_TYPE (mm)
#define SBX_ATTR_APERTURE     "Media:Aperture"    // B_FLOAT_TYPE (f-number)
#define SBX_ATTR_SHUTTER      "Media:Shutter"     // B_FLOAT_TYPE (seconds)
#define SBX_ATTR_GPS          "Media:GPS"         // B_STRING_TYPE "lat,lon"

// User-set
#define SBX_ATTR_RATING  "Photo:Rating"  // B_INT32_TYPE 0..5
#define SBX_ATTR_FLAG    "Photo:Flag"    // B_INT32_TYPE -1/0/+1
#define SBX_ATTR_TAGS    "Photo:Tags"    // B_STRING_TYPE comma-separated
#define SBX_ATTR_ALBUM   "Photo:Album"   // B_STRING_TYPE
#define SBX_ATTR_CAPTION "Photo:Caption" // B_STRING_TYPE

// App-private
#define SBX_ATTR_THUMB256 "Shoebox:Thumb256" // B_RAW_TYPE (PNG bytes)

#endif
