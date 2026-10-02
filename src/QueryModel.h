#ifndef SHOEBOX_QUERY_MODEL_H
#define SHOEBOX_QUERY_MODEL_H

#include <Entry.h>
#include <Handler.h>
#include <Messenger.h>
#include <ObjectList.h>

class BQuery;

class QueryModel : public BHandler {
public:
							QueryModel();
	virtual					~QueryModel();

			void			SetListener(BMessenger listener);
			void			SetPredicate(const char* predicate);
			void			Clear();

			int32			CountEntries() const;
	const	entry_ref*		EntryAt(int32 index) const;

	virtual	void			MessageReceived(BMessage* message);

private:
			void			_HandleQueryUpdate(BMessage* message);
			int32			_IndexOfRef(const entry_ref& ref) const;
			void			_NotifyChanged();

			BQuery*						fQuery;
			BObjectList<entry_ref, true>	fEntries;
			BMessenger					fListener;
};

#endif
