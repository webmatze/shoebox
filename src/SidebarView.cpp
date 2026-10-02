#include "SidebarView.h"

#include "Messages.h"
#include "SmartFolders.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <MenuItem.h>
#include <ObjectList.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <Window.h>

QueryItem::QueryItem(const char* label, const char* predicate)
	: BStringItem(label),
	  fPredicate(predicate),
	  fUserDefined(false)
{
}


QueryItem::QueryItem(const char* label, const char* predicate,
	const entry_ref& ref)
	: BStringItem(label),
	  fPredicate(predicate),
	  fUserDefined(true),
	  fRef(ref)
{
}


QueryListView::QueryListView()
	: BOutlineListView("queries")
{
}


void
QueryListView::SetContextTarget(BMessenger target)
{
	fContextTarget = target;
}


void
QueryListView::MouseDown(BPoint where)
{
	int32 buttons = 0;
	BMessage* current
		= Window() != nullptr ? Window()->CurrentMessage() : nullptr;
	if (current != nullptr)
		current->FindInt32("buttons", &buttons);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		int32 idx = IndexOf(where);
		if (idx >= 0) {
			QueryItem* qi = dynamic_cast<QueryItem*>(ItemAt(idx));
			if (qi != nullptr && qi->IsUserDefined()) {
				Select(idx);
				_ShowContextMenu(where, qi);
				return;
			}
		}
	}
	BOutlineListView::MouseDown(where);
}


void
QueryListView::_ShowContextMenu(BPoint where, QueryItem* item)
{
	BPopUpMenu* menu = new BPopUpMenu("ctx", false, false);

	BMessage* edit = new BMessage(SBX_EDIT_SMART_FOLDER);
	edit->AddRef("ref", &item->Ref());
	menu->AddItem(new BMenuItem("Edit" B_UTF8_ELLIPSIS, edit));

	BMessage* del = new BMessage(SBX_DELETE_SMART_FOLDER);
	del->AddRef("ref", &item->Ref());
	menu->AddItem(new BMenuItem("Delete", del));

	menu->SetTargetForItems(fContextTarget);
	menu->SetAsyncAutoDestruct(true);
	menu->Go(ConvertToScreen(where), true, true, true);
}


SidebarView::SidebarView()
	: BView("sidebar", B_WILL_DRAW)
{
	fList = new QueryListView();

	fSmartFoldersHeader = new BStringItem("Smart Folders");
	fList->AddItem(fSmartFoldersHeader);
	fList->AddUnder(new QueryItem("All Photos",
		"BEOS:TYPE==\"image/*\""), fSmartFoldersHeader);
	fList->AddUnder(new QueryItem("Rated 4+",
		"(BEOS:TYPE==\"image/*\")&&(Photo:Rating>=4)"), fSmartFoldersHeader);
	fList->AddUnder(new QueryItem("Flagged",
		"(BEOS:TYPE==\"image/*\")&&(Photo:Flag==1)"), fSmartFoldersHeader);
	fList->AddUnder(new QueryItem("This Week",
		"(BEOS:TYPE==\"image/*\")&&(last_modified>=THIS_WEEK)"),
		fSmartFoldersHeader);

	fAlbumsHeader = new BStringItem("Albums");
	fList->AddItem(fAlbumsHeader);
	fTagsHeader = new BStringItem("Tags");
	fList->AddItem(fTagsHeader);

	fList->SetSelectionMessage(new BMessage(SBX_SELECT_QUERY));

	BScrollView* scroll
		= new BScrollView("scroll", fList, 0, false, true, B_NO_BORDER);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(scroll);
}


void
SidebarView::SetTarget(BMessenger target)
{
	fList->SetTarget(target);
}


void
SidebarView::SetContextTarget(BMessenger target)
{
	fList->SetContextTarget(target);
}


const char*
SidebarView::SelectedPredicate() const
{
	int32 index = fList->CurrentSelection();
	if (index < 0)
		return nullptr;
	BListItem* item = fList->ItemAt(index);
	QueryItem* qi = dynamic_cast<QueryItem*>(item);
	return qi != nullptr ? qi->Predicate() : nullptr;
}


void
SidebarView::SetCatalog(const BStringList& albums, const BStringList& tags)
{
	_RemoveChildrenOf(fAlbumsHeader);
	for (int32 i = 0; i < albums.CountStrings(); i++) {
		BString name = albums.StringAt(i);
		BString predicate;
		predicate << "(BEOS:TYPE==\"image/*\")&&(Photo:Album==\""
			<< name << "\")";
		fList->AddUnder(new QueryItem(name.String(), predicate.String()),
			fAlbumsHeader);
	}

	_RemoveChildrenOf(fTagsHeader);
	for (int32 i = 0; i < tags.CountStrings(); i++) {
		BString name = tags.StringAt(i);
		BString predicate;
		predicate << "(BEOS:TYPE==\"image/*\")&&(Photo:Tags==\"*"
			<< name << "*\")";
		fList->AddUnder(new QueryItem(name.String(), predicate.String()),
			fTagsHeader);
	}
}


void
SidebarView::SetUserSmartFolders(
	const BObjectList<SmartFolders::Entry, true>& entries)
{
	_RemoveChildrenOf(fSmartFoldersHeader, true);
	for (int32 i = 0; i < entries.CountItems(); i++) {
		const SmartFolders::Entry* e = entries.ItemAt(i);
		fList->AddUnder(new QueryItem(e->name.String(),
				e->predicate.String(), e->file),
			fSmartFoldersHeader);
	}
}


void
SidebarView::_RemoveChildrenOf(BListItem* header, bool userDefinedOnly)
{
	for (int32 i = fList->FullListCountItems() - 1; i >= 0; i--) {
		BListItem* item = fList->FullListItemAt(i);
		if (fList->Superitem(item) != header)
			continue;
		if (userDefinedOnly) {
			QueryItem* qi = dynamic_cast<QueryItem*>(item);
			if (qi == nullptr || !qi->IsUserDefined())
				continue;
		}
		fList->RemoveItem(item);
		delete item;
	}
}
