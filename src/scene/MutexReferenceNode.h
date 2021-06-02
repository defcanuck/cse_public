#pragma once

#include "scene/ReferenceNode.h"

namespace cs
{
	class SceneReferenceHandle;

	CLASS_DEFINITION_REFLECT(ReferenceHandleChoice)
	public:
		ReferenceHandleChoice();

		std::shared_ptr<SceneReferenceHandle> referenceHandle;
		int32 chance;
	};
	
	typedef std::vector<ReferenceHandleChoicePtr> ReferenceHandleChoiceList;

	CLASS_DEFINITION_DERIVED_REFLECT(MutexReferenceNode, ReferenceNode)
	public:
		MutexReferenceNode(const std::string& n, ECSContext* cxt);
		MutexReferenceNode(const std::string& n, ECSContext* cxt, const std::string& refName);

		virtual ~MutexReferenceNode() { }

		virtual std::shared_ptr<SceneReferenceHandle>& getReferenceHandle();

	protected:

		MutexReferenceNode() 
			: ReferenceNode("", nullptr)
			, chanceIndex(-1)
		{ }

		ReferenceHandleChoiceList choices;

	private:

		void calculateChanceIndex();

		int chanceIndex;
	};

}