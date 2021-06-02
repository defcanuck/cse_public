#include "PCH.h"

#include "scene/SceneDebug.h"

namespace cs
{
	namespace SceneDebug
	{

		void setRenderFlag(ScenePtr& scene, const int32& value)
		{
		if (value < 0 || value >= SceneRenderMAX)
			{
				log::error("Invalid render flag");
				return;
			}

			SceneRender flag = (SceneRender)value;
			scene->renderMask.toggle(flag);

			log::info("Toggle render flag: ", kSceneRenderFlag[value]);
		}
	}
}