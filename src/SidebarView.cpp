#include "SidebarView.h"

#include "Messages.h"

#include <LayoutBuilder.h>
#include <Message.h>
#include <OutlineListView.h>
#include <ScrollView.h>

QueryItem::QueryItem(const char* label, const char* predicate)
	: BStringItem(label),
	  fPredicate(predicate)
{
}


SidebarView::SidebarView()
	: BView("sidebar", B_WILL_DRAW)
{
	fList = new BOutlineListView("queries");

	BStringItem* smart = new BStringItem("Smart Folders");
	fList->AddItem(smart);
	fList->AddUnder(new QueryItem("All Photos",
		"BEOS:TYPE==\"image/*\""), smart);
	fList->AddUnder(new QueryItem("Rated 4+",
		"(BEOS:TYPE==\"image/*\")&&(Photo:Rating>=4)"), smart);
	fList->AddUnder(new QueryItem("Flagged",
		"(BEOS:TYPE==\"image/*\")&&(Photo:Flag==1)"), smart);
	fList->AddUnder(new QueryItem("This Week",
		"(BEOS:TYPE==\"image/*\")&&(last_modified>=THIS_WEEK)"), smart);

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
		// Lossy substring match for now; precise matching needs a different
		// storage scheme (per-tag attribute or sentinel delimiters).
		BString predicate;
		predicate << "(BEOS:TYPE==\"image/*\")&&(Photo:Tags==\"*"
			<< name << "*\")";
		fList->AddUnder(new QueryItem(name.String(), predicate.String()),
			fTagsHeader);
	}
}


void
SidebarView::_RemoveChildrenOf(BListItem* header)
{
	for (int32 i = fList->FullListCountItems() - 1; i >= 0; i--) {
		BListItem* item = fList->FullListItemAt(i);
		if (fList->Superitem(item) == header) {
			fList->RemoveItem(item);
			delete item;
		}
	}
}
