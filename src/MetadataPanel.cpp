#include "MetadataPanel.h"

#include "AttributeNames.h"
#include "Messages.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <Node.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

#include <stdlib.h>
#include <string.h>

MetadataPanel::MetadataPanel()
	: BView("metadata", B_WILL_DRAW),
	  fHasSelection(false)
{
	fFilename = new BStringView("filename", "No selection");
	fRating   = new BTextControl("rating",  "Rating:",  "",
		new BMessage(SBX_SET_RATING));
	fTags     = new BTextControl("tags",    "Tags:",    "",
		new BMessage(SBX_SET_TAGS));
	fAlbum    = new BTextControl("album",   "Album:",   "",
		new BMessage(SBX_SET_ALBUM));
	fCaption  = new BTextControl("caption", "Caption:", "",
		new BMessage(SBX_SET_CAPTION));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fFilename)
		.Add(fRating)
		.Add(fTags)
		.Add(fAlbum)
		.Add(fCaption)
		.AddGlue();

	_SetEnabled(false);
}


void
MetadataPanel::AttachedToWindow()
{
	BView::AttachedToWindow();
	fRating->SetTarget(this);
	fTags->SetTarget(this);
	fAlbum->SetTarget(this);
	fCaption->SetTarget(this);
}


void
MetadataPanel::SetRef(const entry_ref& ref)
{
	fCurrentRef = ref;
	fHasSelection = true;

	fFilename->SetText(ref.name);

	BNode node(&ref);
	if (node.InitCheck() != B_OK) {
		Clear();
		return;
	}

	int32 rating = 0;
	node.ReadAttr(SBX_ATTR_RATING, B_INT32_TYPE, 0, &rating, sizeof(rating));
	BString r;
	r << rating;
	fRating->SetText(r.String());

	char buf[1024];
	ssize_t n;

	n = node.ReadAttr(SBX_ATTR_TAGS, B_STRING_TYPE, 0,
		buf, sizeof(buf) - 1);
	buf[n > 0 ? n : 0] = '\0';
	fTags->SetText(buf);

	n = node.ReadAttr(SBX_ATTR_ALBUM, B_STRING_TYPE, 0,
		buf, sizeof(buf) - 1);
	buf[n > 0 ? n : 0] = '\0';
	fAlbum->SetText(buf);

	n = node.ReadAttr(SBX_ATTR_CAPTION, B_STRING_TYPE, 0,
		buf, sizeof(buf) - 1);
	buf[n > 0 ? n : 0] = '\0';
	fCaption->SetText(buf);

	_SetEnabled(true);
}


void
MetadataPanel::Clear()
{
	fHasSelection = false;
	fFilename->SetText("No selection");
	fRating->SetText("");
	fTags->SetText("");
	fAlbum->SetText("");
	fCaption->SetText("");
	_SetEnabled(false);
}


void
MetadataPanel::MessageReceived(BMessage* message)
{
	if (!fHasSelection) {
		BView::MessageReceived(message);
		return;
	}

	BNode node(&fCurrentRef);
	if (node.InitCheck() != B_OK) {
		BView::MessageReceived(message);
		return;
	}

	switch (message->what) {
		case SBX_SET_RATING: {
			int32 v = atoi(fRating->Text());
			if (v < 0) v = 0;
			if (v > 5) v = 5;
			if (v == 0)
				node.RemoveAttr(SBX_ATTR_RATING);
			else
				node.WriteAttr(SBX_ATTR_RATING, B_INT32_TYPE, 0,
					&v, sizeof(v));
			BString s;
			s << v;
			fRating->SetText(s.String());
			return;
		}
		case SBX_SET_TAGS:
			_WriteString(node, SBX_ATTR_TAGS, fTags->Text());
			if (Window() != nullptr)
				Window()->PostMessage(SBX_REFRESH_CATALOG);
			return;
		case SBX_SET_ALBUM:
			_WriteString(node, SBX_ATTR_ALBUM, fAlbum->Text());
			if (Window() != nullptr)
				Window()->PostMessage(SBX_REFRESH_CATALOG);
			return;
		case SBX_SET_CAPTION:
			_WriteString(node, SBX_ATTR_CAPTION, fCaption->Text());
			return;
		default:
			BView::MessageReceived(message);
	}
}


void
MetadataPanel::_SetEnabled(bool enabled)
{
	fRating->SetEnabled(enabled);
	fTags->SetEnabled(enabled);
	fAlbum->SetEnabled(enabled);
	fCaption->SetEnabled(enabled);
}


void
MetadataPanel::_WriteString(BNode& node, const char* attr,
	const char* value)
{
	if (value == nullptr || *value == '\0')
		node.RemoveAttr(attr);
	else
		node.WriteAttr(attr, B_STRING_TYPE, 0,
			value, strlen(value) + 1);
}
