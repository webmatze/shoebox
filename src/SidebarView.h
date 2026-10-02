#ifndef SHOEBOX_SIDEBAR_VIEW_H
#define SHOEBOX_SIDEBAR_VIEW_H

#include <Messenger.h>
#include <String.h>
#include <StringItem.h>
#include <StringList.h>
#include <View.h>

class BOutlineListView;

class QueryItem : public BStringItem {
public:
					QueryItem(const char* label, const char* predicate);

	const char*		Predicate() const { return fPredicate.String(); }

private:
	BString			fPredicate;
};


class SidebarView : public BView {
public:
							SidebarView();

			void			SetTarget(BMessenger target);
	const	char*			SelectedPredicate() const;
			void			SetCatalog(const BStringList& albums,
								const BStringList& tags);

private:
			void			_RemoveChildrenOf(BListItem* header);

			BOutlineListView*	fList;
			BListItem*			fAlbumsHeader;
			BListItem*			fTagsHeader;
};

#endif
