#include "MainWindow.h"

#include "AttributeNames.h"
#include "Importer.h"
#include "Messages.h"
#include "MetadataPanel.h"
#include "PhotoGridView.h"
#include "QueryModel.h"
#include "SidebarView.h"
#include "SmartFolderDialog.h"
#include "SmartFolders.h"
#include "TagModel.h"

#include <Alert.h>
#include <Application.h>
#include <Entry.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <Node.h>
#include <Path.h>
#include <Roster.h>

#include <stdio.h>
#include <time.h>

namespace {

BMenuItem*
make_rate_item(const char* label, int32 value)
{
	BMessage* msg = new BMessage(SBX_RATE_SELECTED);
	msg->AddInt32("value", value);
	return new BMenuItem(label, msg);
}

BMenuItem*
make_flag_item(const char* label, int32 value)
{
	BMessage* msg = new BMessage(SBX_FLAG_SELECTED);
	msg->AddInt32("value", value);
	return new BMenuItem(label, msg);
}

} // namespace

MainWindow::MainWindow()
	: BWindow(BRect(100, 100, 1100, 800), "Shoebox",
		B_DOCUMENT_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
	  fHasSelection(false)
{
	fMenuBar = new BMenuBar("menubar");

	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("New smart folder" B_UTF8_ELLIPSIS,
		new BMessage(SBX_NEW_SMART_FOLDER), 'N'));
	fileMenu->AddItem(new BMenuItem("Import folder" B_UTF8_ELLIPSIS,
		new BMessage(SBX_SHOW_IMPORT_PANEL), 'I'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fileMenu);

	fImageMenu = new BMenu("Image");

	BMenu* rateMenu = new BMenu("Rate");
	rateMenu->AddItem(make_rate_item("No rating",   0));
	rateMenu->AddItem(make_rate_item("1 star",      1));
	rateMenu->AddItem(make_rate_item("2 stars",     2));
	rateMenu->AddItem(make_rate_item("3 stars",     3));
	rateMenu->AddItem(make_rate_item("4 stars",     4));
	rateMenu->AddItem(make_rate_item("5 stars",     5));
	fImageMenu->AddItem(rateMenu);

	BMenu* flagMenu = new BMenu("Flag");
	flagMenu->AddItem(make_flag_item("Pick",   1));
	flagMenu->AddItem(make_flag_item("None",   0));
	flagMenu->AddItem(make_flag_item("Reject", -1));
	fImageMenu->AddItem(flagMenu);

	fImageMenu->AddSeparatorItem();
	fImageMenu->AddItem(new BMenuItem("Open",
		new BMessage(SBX_OPEN_IMAGE), 'O'));
	fImageMenu->AddItem(new BMenuItem("Reveal in Tracker",
		new BMessage(SBX_REVEAL_IMAGE), 'R'));

	fMenuBar->AddItem(fImageMenu);
	fImageMenu->SetEnabled(false);

	fSidebar  = new SidebarView();
	fGrid     = new PhotoGridView();
	fMetadata = new MetadataPanel();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.AddSplit(B_HORIZONTAL, 0)
			.Add(fSidebar,  0.18f)
			.Add(fGrid,     0.62f)
			.Add(fMetadata, 0.20f)
		.End();

	fQueryModel = new QueryModel();
	AddHandler(fQueryModel);
	fQueryModel->SetListener(BMessenger(fGrid, this));

	fSidebar->SetTarget(BMessenger(this));
	fSidebar->SetContextTarget(BMessenger(this));

	BMessenger me(this);
	fImportPanel = new BFilePanel(B_OPEN_PANEL, &me, nullptr,
		B_DIRECTORY_NODE, false, new BMessage(SBX_IMPORT_REFS));
	fImportPanel->Window()->SetTitle("Shoebox: import folder");

	_RefreshCatalog();
	_RefreshSmartFolders();
}


MainWindow::~MainWindow()
{
	Lock();
	RemoveHandler(fQueryModel);
	Unlock();
	delete fQueryModel;
	delete fImportPanel;
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case SBX_SELECT_QUERY:
			_OnSelectQuery();
			return;
		case SBX_SHOW_IMPORT_PANEL:
			fImportPanel->Show();
			return;
		case SBX_IMPORT_REFS:
			_OnImportRefs(message);
			return;
		case SBX_SELECTION_CHANGED:
			_OnSelectionChanged(message);
			return;
		case SBX_RATE_SELECTED: {
			int32 v = 0;
			message->FindInt32("value", &v);
			_OnRate(v);
			return;
		}
		case SBX_FLAG_SELECTED: {
			int32 v = 0;
			message->FindInt32("value", &v);
			_OnFlag(v);
			return;
		}
		case SBX_OPEN_IMAGE:
			_OnOpenImage();
			return;
		case SBX_REVEAL_IMAGE:
			_OnRevealImage();
			return;
		case SBX_REFRESH_CATALOG:
			_RefreshCatalog();
			return;
		case SBX_NEW_SMART_FOLDER:
			_OnNewSmartFolder();
			return;
		case SBX_EDIT_SMART_FOLDER:
			_OnEditSmartFolder(message);
			return;
		case SBX_DELETE_SMART_FOLDER:
			_OnDeleteSmartFolder(message);
			return;
		case SBX_SAVE_SMART_FOLDER:
			_OnSaveSmartFolder(message);
			return;
		default:
			BWindow::MessageReceived(message);
	}
}


void
MainWindow::_OnSelectQuery()
{
	const char* predicate = fSidebar->SelectedPredicate();
	fprintf(stderr, "[sel] predicate=%s\n",
		predicate != nullptr ? predicate : "(null)");
	if (predicate == nullptr)
		return;

	BString expanded = _ExpandPredicate(predicate);
	fprintf(stderr, "[sel] expanded=%s\n", expanded.String());
	fQueryModel->SetPredicate(expanded.String());
}


void
MainWindow::_OnSelectionChanged(const BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("ref", &ref) == B_OK) {
		fSelectedRef = ref;
		fHasSelection = true;
		fMetadata->SetRef(ref);
	} else {
		fHasSelection = false;
		fMetadata->Clear();
	}
	fImageMenu->SetEnabled(fHasSelection);
}


void
MainWindow::_OnRate(int32 value)
{
	if (!fHasSelection)
		return;
	BNode node(&fSelectedRef);
	if (node.InitCheck() != B_OK)
		return;
	if (value == 0)
		node.RemoveAttr(SBX_ATTR_RATING);
	else
		node.WriteAttr(SBX_ATTR_RATING, B_INT32_TYPE, 0,
			&value, sizeof(value));
	fMetadata->SetRef(fSelectedRef); // refresh displayed values
}


void
MainWindow::_OnFlag(int32 value)
{
	if (!fHasSelection)
		return;
	BNode node(&fSelectedRef);
	if (node.InitCheck() != B_OK)
		return;
	if (value == 0)
		node.RemoveAttr(SBX_ATTR_FLAG);
	else
		node.WriteAttr(SBX_ATTR_FLAG, B_INT32_TYPE, 0,
			&value, sizeof(value));
}


void
MainWindow::_OnOpenImage()
{
	if (!fHasSelection)
		return;
	be_roster->Launch(&fSelectedRef);
}


void
MainWindow::_OnRevealImage()
{
	if (!fHasSelection)
		return;
	BEntry entry(&fSelectedRef);
	BEntry parent;
	if (entry.GetParent(&parent) != B_OK)
		return;
	entry_ref parentRef;
	if (parent.GetRef(&parentRef) != B_OK)
		return;
	be_roster->Launch(&parentRef);
}


void
MainWindow::_OnImportRefs(const BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("refs", &ref) != B_OK)
		return;

	Importer::Result r = Importer::ImportFolder(ref);
	_RefreshCatalog();

	char text[160];
	snprintf(text, sizeof(text),
		"Scanned %d files.\n%d images re-indexed.\n%d errors.",
		(int)r.scanned, (int)r.reindexed, (int)r.errors);
	BAlert* alert = new BAlert("Import complete", text, "OK");
	alert->Go(nullptr); // async; don't block the window thread
}


void
MainWindow::_RefreshCatalog()
{
	TagModel::Catalog cat;
	TagModel::Collect(cat);
	fSidebar->SetCatalog(cat.albums, cat.tags);
}


void
MainWindow::_RefreshSmartFolders()
{
	BObjectList<SmartFolders::Entry, true> entries(16);
	SmartFolders::Load(entries);
	fSidebar->SetUserSmartFolders(entries);
}


void
MainWindow::_OnNewSmartFolder()
{
	SmartFolderDialog* dlg = new SmartFolderDialog(BMessenger(this),
		nullptr, "", "");
	dlg->Show();
}


void
MainWindow::_OnEditSmartFolder(const BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("ref", &ref) != B_OK)
		return;

	// Reload current values from disk.
	BObjectList<SmartFolders::Entry, true> entries(16);
	SmartFolders::Load(entries);
	for (int32 i = 0; i < entries.CountItems(); i++) {
		const SmartFolders::Entry* e = entries.ItemAt(i);
		if (e->file == ref) {
			SmartFolderDialog* dlg = new SmartFolderDialog(
				BMessenger(this), &ref,
				e->name.String(), e->predicate.String());
			dlg->Show();
			return;
		}
	}
}


void
MainWindow::_OnDeleteSmartFolder(const BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("ref", &ref) != B_OK)
		return;

	BAlert* alert = new BAlert("Delete smart folder",
		"Delete this smart folder?", "Cancel", "Delete",
		nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	int32 choice = alert->Go();
	if (choice != 1)
		return;

	SmartFolders::Delete(ref);
	_RefreshSmartFolders();
}


void
MainWindow::_OnSaveSmartFolder(const BMessage* message)
{
	const char* name = nullptr;
	const char* predicate = nullptr;
	message->FindString("name", &name);
	message->FindString("predicate", &predicate);
	if (name == nullptr || predicate == nullptr)
		return;

	entry_ref existing;
	bool isEdit = (message->FindRef("existing", &existing) == B_OK);

	status_t s = isEdit
		? SmartFolders::Update(existing, name, predicate)
		: SmartFolders::Create(name, predicate);
	if (s != B_OK) {
		char text[256];
		snprintf(text, sizeof(text),
			"Could not save smart folder: %s", strerror(s));
		BAlert* alert = new BAlert("Save failed", text, "OK");
		alert->Go(nullptr);
		return;
	}
	_RefreshSmartFolders();
}


BString
MainWindow::_ExpandPredicate(const char* raw) const
{
	BString s(raw);
	if (s.FindFirst("THIS_WEEK") >= 0) {
		time_t oneWeekAgo = time(nullptr) - 7 * 24 * 60 * 60;
		BString ts;
		ts << (int64)oneWeekAgo;
		s.ReplaceAll("THIS_WEEK", ts.String());
	}
	return s;
}
