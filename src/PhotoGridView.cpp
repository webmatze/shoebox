#include "PhotoGridView.h"

#include "Messages.h"
#include "ThumbnailCache.h"

#include <Bitmap.h>
#include <Entry.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <ObjectList.h>
#include <ScrollView.h>
#include <Window.h>


class GridContent : public BView {
public:
							GridContent();
	virtual					~GridContent();

	virtual	void			Draw(BRect updateRect);
	virtual	void			FrameResized(float width, float height);
	virtual	void			MouseDown(BPoint where);

			void			SetRefs(const BMessage& message);

private:
	struct Tile {
		entry_ref	ref;
		BBitmap*	thumb; // owned
	};

	static const int32		kTile = 160;
	static const int32		kGap  = 12;

			int32			_Columns() const;
			BRect			_TileRect(int32 index, int32 cols) const;
			void			_InvalidateTile(int32 index, int32 cols);
			void			_Relayout();
			void			_Clear();
			void			_NotifySelection();

			BObjectList<Tile, true>	fTiles;
			int32					fSelected;
};


GridContent::GridContent()
	: BView(BRect(0, 0, 400, 400), "gridContent",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
		B_WILL_DRAW | B_FRAME_EVENTS),
	  fTiles(32),
	  fSelected(-1)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


GridContent::~GridContent()
{
	_Clear();
}


void
GridContent::_Clear()
{
	for (int32 i = 0; i < fTiles.CountItems(); i++)
		delete fTiles.ItemAt(i)->thumb;
	fTiles.MakeEmpty();
}


void
GridContent::SetRefs(const BMessage& message)
{
	entry_ref prev;
	bool hadSelection = (fSelected >= 0);
	if (hadSelection)
		prev = fTiles.ItemAt(fSelected)->ref;

	_Clear();
	fSelected = -1;

	entry_ref ref;
	for (int32 i = 0; message.FindRef("refs", i, &ref) == B_OK; i++) {
		Tile* t = new Tile();
		t->ref = ref;
		t->thumb = ThumbnailCache::LoadOrGenerate(ref);
		fTiles.AddItem(t);
		if (hadSelection && ref == prev)
			fSelected = i;
	}

	_Relayout();
	Invalidate();
	_NotifySelection();
}


int32
GridContent::_Columns() const
{
	float avail = Bounds().Width() - kGap;
	int32 cols = (int32)((avail + kGap) / (kTile + kGap));
	return cols < 1 ? 1 : cols;
}


BRect
GridContent::_TileRect(int32 index, int32 cols) const
{
	int32 col = index % cols;
	int32 row = index / cols;
	float x = kGap + col * (kTile + kGap);
	float y = kGap + row * (kTile + kGap);
	return BRect(x, y, x + kTile - 1, y + kTile - 1);
}


void
GridContent::_InvalidateTile(int32 index, int32 cols)
{
	if (index < 0)
		return;
	BRect r = _TileRect(index, cols);
	r.InsetBy(-3, -3); // include selection frame
	Invalidate(r);
}


void
GridContent::MouseDown(BPoint where)
{
	int32 clicks = 1;
	BMessage* current = Window() != nullptr ? Window()->CurrentMessage() : nullptr;
	if (current != nullptr)
		current->FindInt32("clicks", &clicks);

	int32 cols = _Columns();
	for (int32 i = 0; i < fTiles.CountItems(); i++) {
		if (_TileRect(i, cols).Contains(where)) {
			if (fSelected != i) {
				int32 old = fSelected;
				fSelected = i;
				_InvalidateTile(old, cols);
				_InvalidateTile(i, cols);
				_NotifySelection();
			}
			if (clicks >= 2 && Window() != nullptr) {
				BMessage open(SBX_OPEN_IMAGE);
				Window()->PostMessage(&open);
			}
			return;
		}
	}
	if (fSelected >= 0) {
		int32 old = fSelected;
		fSelected = -1;
		_InvalidateTile(old, cols);
		_NotifySelection();
	}
}


void
GridContent::_NotifySelection()
{
	BWindow* win = Window();
	if (win == nullptr)
		return;
	BMessage msg(SBX_SELECTION_CHANGED);
	if (fSelected >= 0)
		msg.AddRef("ref", &fTiles.ItemAt(fSelected)->ref);
	win->PostMessage(&msg);
}


void
GridContent::_Relayout()
{
	int32 cols = _Columns();
	int32 rows = (fTiles.CountItems() + cols - 1) / cols;
	if (rows < 1) rows = 1;

	float h = kGap + rows * (kTile + kGap);
	if (h < Bounds().Height())
		h = Bounds().Height();

	ResizeTo(Bounds().Width(), h);
}


void
GridContent::FrameResized(float, float)
{
	_Relayout();
	Invalidate();
}


void
GridContent::Draw(BRect updateRect)
{
	int32 cols = _Columns();
	for (int32 i = 0; i < fTiles.CountItems(); i++) {
		BRect tile = _TileRect(i, cols);
		if (!tile.Intersects(updateRect))
			continue;

		Tile* t = fTiles.ItemAt(i);
		if (t->thumb != nullptr) {
			BRect src = t->thumb->Bounds();
			float sw = src.Width() + 1.0f;
			float sh = src.Height() + 1.0f;
			float scale = (sw > sh) ? (kTile / sw) : (kTile / sh);
			float dw = sw * scale;
			float dh = sh * scale;
			BRect dst = tile;
			dst.InsetBy((kTile - dw) / 2.0f, (kTile - dh) / 2.0f);
			DrawBitmap(t->thumb, src, dst);
		} else {
			SetHighColor(220, 220, 220);
			FillRect(tile);
			SetHighColor(120, 120, 120);
			DrawString("?",
				BPoint(tile.left + kTile / 2 - 4, tile.top + kTile / 2 + 4));
		}

		if (i == fSelected) {
			SetHighColor(ui_color(B_CONTROL_HIGHLIGHT_COLOR));
			SetPenSize(3);
			StrokeRect(tile.InsetByCopy(-2, -2));
			SetPenSize(1);
		}
	}
}


PhotoGridView::PhotoGridView()
	: BView("grid", B_WILL_DRAW)
{
	fContent = new GridContent();

	BScrollView* scroll = new BScrollView("gridScroll", fContent, 0,
		false, true, B_NO_BORDER);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(scroll);
}


PhotoGridView::~PhotoGridView()
{
}


void
PhotoGridView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case SBX_QUERY_CHANGED:
			fContent->SetRefs(*message);
			return;
		default:
			BView::MessageReceived(message);
	}
}
