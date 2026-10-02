#include "SmartFolderDialog.h"

#include "Messages.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <String.h>
#include <TextControl.h>

namespace {

enum {
	kMsgSave   = 'sfSv',
	kMsgCancel = 'sfCn',
};

} // namespace


SmartFolderDialog::SmartFolderDialog(BMessenger target,
	const entry_ref* existing,
	const char* presetName, const char* presetPredicate)
	: BWindow(BRect(200, 200, 650, 360),
		existing != nullptr ? "Edit smart folder" : "New smart folder",
		B_TITLED_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS
			| B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
	  fTarget(target),
	  fIsEdit(existing != nullptr)
{
	if (fIsEdit)
		fExistingRef = *existing;

	fName = new BTextControl("name", "Name:",
		presetName != nullptr ? presetName : "", nullptr);
	fPredicate = new BTextControl("predicate", "Predicate:",
		presetPredicate != nullptr ? presetPredicate : "", nullptr);

	BButton* save = new BButton("save", "Save",
		new BMessage(kMsgSave));
	save->MakeDefault(true);
	BButton* cancel = new BButton("cancel", "Cancel",
		new BMessage(kMsgCancel));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fName)
		.Add(fPredicate)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancel)
			.Add(save)
		.End();
}


void
SmartFolderDialog::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgSave:
			_OnSave();
			return;
		case kMsgCancel:
			PostMessage(B_QUIT_REQUESTED);
			return;
		default:
			BWindow::MessageReceived(message);
	}
}


void
SmartFolderDialog::_OnSave()
{
	BString name(fName->Text());
	BString predicate(fPredicate->Text());
	name.Trim();
	predicate.Trim();

	if (name.Length() == 0 || predicate.Length() == 0) {
		BAlert* alert = new BAlert("Missing fields",
			"Name and predicate are both required.", "OK");
		alert->Go(nullptr);
		return;
	}

	BMessage out(SBX_SAVE_SMART_FOLDER);
	out.AddString("name", name);
	out.AddString("predicate", predicate);
	if (fIsEdit)
		out.AddRef("existing", &fExistingRef);
	fTarget.SendMessage(&out);

	PostMessage(B_QUIT_REQUESTED);
}
