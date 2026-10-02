#include "ThumbnailCache.h"

#include "AttributeNames.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <DataIO.h>
#include <Node.h>
#include <fs_attr.h>
#include <TranslationUtils.h>
#include <TranslatorFormats.h>
#include <TranslatorRoster.h>
#include <View.h>

#include <stdio.h>

namespace {

BBitmap*
scale_bitmap(BBitmap* source, int32 maxDim)
{
	BRect sb = source->Bounds();
	float sw = sb.Width() + 1.0f;
	float sh = sb.Height() + 1.0f;
	float scale = (sw > sh) ? (maxDim / sw) : (maxDim / sh);
	if (scale > 1.0f)
		scale = 1.0f; // never upscale

	int32 dw = (int32)(sw * scale);
	int32 dh = (int32)(sh * scale);
	if (dw < 1) dw = 1;
	if (dh < 1) dh = 1;

	BBitmap* thumb
		= new BBitmap(BRect(0, 0, dw - 1, dh - 1), B_RGB32, true);
	if (thumb->InitCheck() != B_OK) {
		delete thumb;
		return nullptr;
	}

	BView* drawer
		= new BView(thumb->Bounds(), "scale", B_FOLLOW_NONE, B_WILL_DRAW);
	thumb->AddChild(drawer);
	thumb->Lock();
	drawer->SetDrawingMode(B_OP_COPY);
	drawer->DrawBitmap(source, source->Bounds(), thumb->Bounds());
	drawer->Sync();
	thumb->Unlock();
	thumb->RemoveChild(drawer);
	delete drawer;
	return thumb;
}


status_t
encode_png(BBitmap* bitmap, BMallocIO& out)
{
	BBitmapStream stream(bitmap);
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	status_t s = roster->Translate(&stream, nullptr, nullptr, &out,
		B_PNG_FORMAT);
	BBitmap* detached = nullptr;
	stream.DetachBitmap(&detached); // keep ownership with caller
	return s;
}


BBitmap*
decode_png(const void* data, size_t size)
{
	BMemoryIO memory(data, size);
	return BTranslationUtils::GetBitmap(&memory);
}

} // namespace


BBitmap*
ThumbnailCache::LoadOrGenerate(const entry_ref& ref)
{
	BNode node(&ref);
	if (node.InitCheck() != B_OK)
		return nullptr;

	attr_info info;
	if (node.GetAttrInfo(SBX_ATTR_THUMB256, &info) == B_OK && info.size > 0) {
		char* buf = new char[info.size];
		ssize_t r = node.ReadAttr(SBX_ATTR_THUMB256, B_RAW_TYPE, 0,
			buf, info.size);
		BBitmap* cached = (r == info.size)
			? decode_png(buf, (size_t)info.size) : nullptr;
		delete[] buf;
		if (cached != nullptr)
			return cached;
	}

	BBitmap* source = BTranslationUtils::GetBitmap(&ref);
	if (source == nullptr) {
		fprintf(stderr, "[thumb] decode failed: %s\n", ref.name);
		return nullptr;
	}

	BBitmap* thumb = scale_bitmap(source, kThumbSize);
	delete source;
	if (thumb == nullptr)
		return nullptr;

	BMallocIO buf;
	if (encode_png(thumb, buf) == B_OK) {
		node.WriteAttr(SBX_ATTR_THUMB256, B_RAW_TYPE, 0,
			buf.Buffer(), buf.BufferLength());
	}
	return thumb;
}
