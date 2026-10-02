#include "IndexSetup.h"

#include "AttributeNames.h"

#include <SupportDefs.h>
#include <TypeConstants.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <fs_index.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace {

struct IndexSpec {
	const char* name;
	uint32		type;
};

const IndexSpec kIndices[] = {
	{ "BEOS:TYPE",           B_MIME_STRING_TYPE },
	{ SBX_ATTR_RATING,       B_INT32_TYPE       },
	{ SBX_ATTR_FLAG,         B_INT32_TYPE       },
	{ SBX_ATTR_TAGS,         B_STRING_TYPE      },
	{ SBX_ATTR_ALBUM,        B_STRING_TYPE      },
	{ SBX_ATTR_CAPTURE_TIME, B_INT32_TYPE       },
	{ SBX_ATTR_CAMERA,       B_STRING_TYPE      },
};

} // namespace


void
SetupShoeboxIndices()
{
	BVolumeRoster roster;
	BVolume volume;
	while (roster.GetNextVolume(&volume) == B_OK) {
		if (!volume.KnowsQuery() || !volume.KnowsAttr() || volume.IsReadOnly())
			continue;

		dev_t dev = volume.Device();
		for (const IndexSpec& spec : kIndices) {
			if (fs_create_index(dev, spec.name, spec.type, 0) == 0) {
				fprintf(stderr, "[idx] created %s on dev=%d\n",
					spec.name, (int)dev);
			} else if (errno != B_FILE_EXISTS && errno != B_NAME_IN_USE) {
				fprintf(stderr, "[idx] skip %s on dev=%d: %s\n",
					spec.name, (int)dev, strerror(errno));
			}
		}
	}
}
