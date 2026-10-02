#ifndef SHOEBOX_METADATA_PANEL_H
#define SHOEBOX_METADATA_PANEL_H

#include <Entry.h>
#include <View.h>

class BNode;
class BStringView;
class BTextControl;

class MetadataPanel : public BView {
public:
							MetadataPanel();

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* message);

			void			SetRef(const entry_ref& ref);
			void			Clear();

private:
			void			_SetEnabled(bool enabled);
			void			_WriteString(BNode& node, const char* attr,
								const char* value);

			BStringView*	fFilename;
			BTextControl*	fRating;
			BTextControl*	fTags;
			BTextControl*	fAlbum;
			BTextControl*	fCaption;

			entry_ref		fCurrentRef;
			bool			fHasSelection;
};

#endif
