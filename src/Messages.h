#ifndef SHOEBOX_MESSAGES_H
#define SHOEBOX_MESSAGES_H

enum {
	SBX_SELECT_QUERY      = 'sbSQ', // sidebar -> window: smart folder chosen
	SBX_QUERY_CHANGED     = 'sbQC', // model   -> grid:   entry list changed
	SBX_SHOW_IMPORT_PANEL = 'sbIp', // menu    -> window: open import panel
	SBX_IMPORT_REFS       = 'sbIr', // panel   -> window: user chose a folder
	SBX_SELECTION_CHANGED = 'sbSC', // grid    -> window: selected ref changed
	SBX_SET_RATING        = 'sbMR', // panel   -> self:   write Photo:Rating
	SBX_SET_TAGS          = 'sbMT', // panel   -> self:   write Photo:Tags
	SBX_SET_ALBUM         = 'sbMA', // panel   -> self:   write Photo:Album
	SBX_SET_CAPTION       = 'sbMC', // panel   -> self:   write Photo:Caption
	SBX_RATE_SELECTED     = 'sbRS', // menu    -> window: rate current (0..5)
	SBX_FLAG_SELECTED     = 'sbFS', // menu    -> window: flag current (-1/0/1)
	SBX_OPEN_IMAGE        = 'sbOI', // menu    -> window: open current in app
	SBX_REVEAL_IMAGE      = 'sbRI', // menu    -> window: reveal in Tracker
	SBX_REFRESH_CATALOG   = 'sbRK', // panel/import -> window: rescan tags/albums
	SBX_NEW_SMART_FOLDER  = 'sbNS', // menu    -> window: show new SF dialog
	SBX_EDIT_SMART_FOLDER = 'sbES', // sidebar -> window: show edit SF dialog
	SBX_DELETE_SMART_FOLDER = 'sbDS', // sidebar -> window: delete SF
	SBX_SAVE_SMART_FOLDER = 'sbSS', // dialog  -> window: persist SF (name,
	                                //          predicate, optional ref)
};

#endif
