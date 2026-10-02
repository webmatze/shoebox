#ifndef SHOEBOX_PHOTO_GRID_VIEW_H
#define SHOEBOX_PHOTO_GRID_VIEW_H

#include <View.h>

class GridContent;

class PhotoGridView : public BView {
public:
							PhotoGridView();
	virtual					~PhotoGridView();

	virtual	void			MessageReceived(BMessage* message);

private:
			GridContent*	fContent;
};

#endif
