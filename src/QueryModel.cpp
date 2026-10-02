#include "QueryModel.h"

#include "Messages.h"

#include <AppDefs.h>
#include <NodeMonitor.h>
#include <Query.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <fs_query.h>

#include <stdio.h>
#include <string.h>

QueryModel::QueryModel()
	: BHandler("QueryModel"),
	  fQuery(nullptr),
	  fEntries(50)
{
}


QueryModel::~QueryModel()
{
	delete fQuery;
}


void
QueryModel::SetListener(BMessenger listener)
{
	fListener = listener;
}


void
QueryModel::Clear()
{
	if (fQuery != nullptr) {
		fQuery->Clear();
		delete fQuery;
		fQuery = nullptr;
	}
	fEntries.MakeEmpty();
}


void
QueryModel::SetPredicate(const char* predicate)
{
	fprintf(stderr, "[qm] SetPredicate: %s\n", predicate);
	Clear();

	BVolumeRoster roster;
	BVolume volume;
	while (roster.GetNextVolume(&volume) == B_OK) {
		if (!volume.KnowsQuery())
			continue;

		char name[B_FILE_NAME_LENGTH] = {0};
		volume.GetName(name);
		fprintf(stderr, "[qm]  volume=\"%s\" dev=%d\n",
			name, (int)volume.Device());

		BQuery* query = new BQuery();
		query->SetVolume(&volume);
		query->SetPredicate(predicate);
		query->SetFlags(B_LIVE_QUERY);
		query->SetTarget(BMessenger(this));

		status_t s = query->Fetch();
		fprintf(stderr, "[qm]  Fetch -> %s\n", strerror(s));
		if (s != B_OK) {
			delete query;
			continue;
		}

		int32 initial = 0;
		entry_ref ref;
		while (query->GetNextRef(&ref) == B_OK) {
			fEntries.AddItem(new entry_ref(ref));
			initial++;
		}
		fprintf(stderr, "[qm]  initial=%d\n", (int)initial);

		fQuery = query;
		break; // only first query-capable volume for now
	}

	fprintf(stderr, "[qm]  total=%d listener=%d\n",
		(int)fEntries.CountItems(), fListener.IsValid() ? 1 : 0);
	_NotifyChanged();
}


int32
QueryModel::CountEntries() const
{
	return fEntries.CountItems();
}


const entry_ref*
QueryModel::EntryAt(int32 index) const
{
	return fEntries.ItemAt(index);
}


void
QueryModel::MessageReceived(BMessage* message)
{
	if (message->what == B_QUERY_UPDATE) {
		_HandleQueryUpdate(message);
		return;
	}
	BHandler::MessageReceived(message);
}


void
QueryModel::_HandleQueryUpdate(BMessage* message)
{
	int32 opcode = 0;
	if (message->FindInt32("opcode", &opcode) != B_OK)
		return;

	int32 device = 0;
	int64 directory = 0;
	const char* name = nullptr;
	message->FindInt32("device", &device);
	message->FindInt64("directory", &directory);
	message->FindString("name", &name);
	if (name == nullptr)
		return;

	entry_ref ref((dev_t)device, (ino_t)directory, name);

	if (opcode == B_ENTRY_CREATED) {
		if (_IndexOfRef(ref) < 0) {
			fEntries.AddItem(new entry_ref(ref));
			_NotifyChanged();
		}
	} else if (opcode == B_ENTRY_REMOVED) {
		int32 index = _IndexOfRef(ref);
		if (index >= 0) {
			fEntries.RemoveItemAt(index);
			_NotifyChanged();
		}
	}
}


int32
QueryModel::_IndexOfRef(const entry_ref& ref) const
{
	for (int32 i = 0; i < fEntries.CountItems(); i++) {
		if (*fEntries.ItemAt(i) == ref)
			return i;
	}
	return -1;
}


void
QueryModel::_NotifyChanged()
{
	if (!fListener.IsValid())
		return;
	BMessage msg(SBX_QUERY_CHANGED);
	msg.AddInt32("count", fEntries.CountItems());
	for (int32 i = 0; i < fEntries.CountItems(); i++)
		msg.AddRef("refs", fEntries.ItemAt(i));
	fListener.SendMessage(&msg);
}
