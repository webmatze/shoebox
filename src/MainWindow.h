#ifndef SHOEBOX_MAIN_WINDOW_H
#define SHOEBOX_MAIN_WINDOW_H

#include <String.h>
#include <Window.h>

class BFilePanel;
class BMenu;
class BMenuBar;
class SidebarView;
class PhotoGridView;
class MetadataPanel;
class QueryModel;

class MainWindow : public BWindow {
public:
							MainWindow();
	virtual					~MainWindow();

	virtual	bool			QuitRequested();
	virtual	void			MessageReceived(BMessage* message);

private:
			void			_OnSelectQuery();
			void			_OnImportRefs(const BMessage* message);
			void			_OnSelectionChanged(const BMessage* message);
			void			_OnRate(int32 value);
			void			_OnFlag(int32 value);
			void			_OnOpenImage();
			void			_OnRevealImage();
			void			_RefreshCatalog();
			BString			_ExpandPredicate(const char* raw) const;

			BMenuBar*		fMenuBar;
			BMenu*			fImageMenu;
			SidebarView*	fSidebar;
			PhotoGridView*	fGrid;
			MetadataPanel*	fMetadata;
			QueryModel*		fQueryModel;
			BFilePanel*		fImportPanel;

			entry_ref		fSelectedRef;
			bool			fHasSelection;
};

#endif
