#include "anim_module.h"
#include "anim_timeline.h"
#include "editor/anim_timeline_editor.h"

namespace Echo
{
	DECLARE_MODULE(AnimModule)

	AnimModule::AnimModule()
	{
	}

	AnimModule* AnimModule::instance()
	{
		static AnimModule* inst = EchoNew(AnimModule);
		return inst;
	}

	void AnimModule::bindMethods()
	{

	}

	void AnimModule::registerTypes()
	{
		Class::registerType<Timeline>();

		REGISTER_OBJECT_EDITOR(Timeline, TimelineEditor)
	}
}