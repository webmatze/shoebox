#ifndef SHOEBOX_SIDEBAR_VIEW_H
#define SHOEBOX_SIDEBAR_VIEW_H

#include <Entry.h>
#include <Messenger.h>
#include <OutlineListView.h>
#include <String.h>
#include <StringItem.h>
#include <StringList.h>
#include <View.h>

namespace SmartFolders { struct Entry; }
template <typename T, bool Owning> class BObjectList;


class QueryItem : public BStringItem {
public:
					QueryItem(const char* label, const char* predicate);
					QueryItem(const char* label, const char* predicate,
						const entry_ref& userDefinedFile);

	const char*		Predicate() const { return fPredicate.String(); }
	bool			IsUserDefined() const { return fUserDefined; }
	const entry_ref& Ref() const { return fRef; }

private:
	BString			fPredicate;
	bool			fUserDefined;
	entry_ref		fRef;
};


class QueryListView : public BOutlineListView {
public:
							QueryListView();

			void			SetContextTarget(BMessenger target);

	virtual	void			MouseDown(BPoint where);

private:
			void			_ShowContextMenu(BPoint where, QueryItem* item);

			BMessenger		fContextTarget;
};


class SidebarView : public BView {
public:
							SidebarView();

			void			SetTarget(BMessenger target);
			void			SetContextTarget(BMessenger target);
	const	char*			SelectedPredicate() const;
			void			SetCatalog(const BStringList& albums,
								const BStringList& tags);
			void			SetUserSmartFolders(
								const BObjectList<SmartFolders::Entry, true>&
									entries);

private:
			void			_RemoveChildrenOf(BListItem* header,
								bool userDefinedOnly = false);

			QueryListView*	fList;
			BListItem*		fSmartFoldersHeader;
			BListItem*		fAlbumsHeader;
			BListItem*		fTagsHeader;
};

#endif
