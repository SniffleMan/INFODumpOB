#include "common/IDebugLog.h"  // IDebugLog
#include "common/ITypes.h"
#include "obse/GameAPI.h"  // DataHandler
#include "obse/GameData.h"  // DataHandler
#include "obse/PluginAPI.h"  // OBSEMessagingInterface

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS
#include <memory>


static PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
static OBSEMessagingInterface*	g_messaging = 0;


void MessageHandler(OBSEMessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case OBSEMessagingInterface::kMessage_SaveGame:
		{
			DataHandler* dataHandler = *g_dataHandler;
			TESForm* form = 0;
			TESTopic* topic = 0;
			NiTLargeArray<TESTopicInfo*>* topicInfoArray = 0;
			for (UInt32 i = 0; i < dataHandler->topics.Count(); ++i) {
				topic = dataHandler->topics.GetNthItem(i);
				_MESSAGE("DIAL: %08X", topic->refID);
				if (topic->firstEntry) {
					std::uintptr_t ptr = (std::uintptr_t)topic->firstEntry;
					ptr += sizeof(void*);
					topicInfoArray = (NiTLargeArray<TESTopicInfo*>*)(ptr);
					for (UInt32 i = 0; i < topicInfoArray->numObjs; ++i) {
						form = (TESForm*)topicInfoArray->data[i];
						_MESSAGE("INFO: %08X", form->refID);
					}
				}
				if (topic->questInfoList) {
					for (TESTopic::QuestInfoEntry* infoEntry = topic->questInfoList; infoEntry; infoEntry = infoEntry->next) {
						std::uintptr_t ptr = (std::uintptr_t)infoEntry->data;
						ptr += sizeof(void*);
						topicInfoArray = (NiTLargeArray<TESTopicInfo*>*)(ptr);
						for (UInt32 i = 0; i < topicInfoArray->numObjs; ++i) {
							form = (TESForm*)topicInfoArray->data[i];
							_MESSAGE("INFO: %08X", form->refID);
						}
					}
				}
			}
		}
		break;
	}
}


extern "C" {
	bool OBSEPlugin_Query(const OBSEInterface* a_obse, PluginInfo* a_info)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Oblivion\\OBSE\\INFODumpOB.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		a_info->infoVersion = PluginInfo::kInfoVersion;
		a_info->name = "INFODumpOB";
		a_info->version = 1;

		g_pluginHandle = a_obse->GetPluginHandle();

		if (a_obse->isEditor) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		if (a_obse->oblivionVersion != OBLIVION_VERSION_1_2_416) {
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", a_obse->oblivionVersion);
			return false;
		}

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface* a_obse)
	{
		g_messaging = (OBSEMessagingInterface*)a_obse->QueryInterface(kInterface_Messaging);
		if (!g_messaging->RegisterListener(g_pluginHandle, "OBSE", MessageHandler)) {
			_FATALERROR("[FATAL ERROR] Messaging interface registration failed!\n");
			return false;
		}

		return true;
	};
}
