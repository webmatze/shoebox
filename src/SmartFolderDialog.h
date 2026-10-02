#ifndef SHOEBOX_SMART_FOLDER_DIALOG_H
#define SHOEBOX_SMART_FOLDER_DIALOG_H

#include <Entry.h>
#include <Messenger.h>
#include <Window.h>

class BTextControl;

class SmartFolderDialog : public BWindow {
public:
	// If `existing` is non-null the dialog opens in edit mode (prefilled).
							SmartFolderDialog(BMessenger target,
								const entry_ref* existing,
								const char* presetName,
								const char* presetPredicate);

	virtual	void			MessageReceived(BMessage* message);

private:
			void			_OnSave();

			BMessenger		fTarget;
			bool			fIsEdit;
			entry_ref		fExistingRef;
			BTextControl*	fName;
			BTextControl*	fPredicate;
};

#endif
