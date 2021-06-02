#include "scene/MutexReferenceNode.h"
#include "scene/SceneReference.h"

namespace cs
{

	DEFINE_META_VECTOR_NEW(ReferenceHandleChoiceList, ReferenceHandleChoice, ReferenceHandleChoiceList);

	BEGIN_META_CLASS(ReferenceHandleChoice)
		ADD_MEMBER_PTR(referenceHandle);
		ADD_MEMBER(chance);
			SET_MEMBER_MIN(0);
			SET_MEMBER_MAX(100);
	END_META()

	BEGIN_META_CLASS(MutexReferenceNode)
		ADD_MEMBER(choices);
			ADD_COMBO_META_LABEL(ReferenceHandleChoice, "Choice");
		ADD_MEMBER_OVERRIDE_FLAGS(children.value_vec, Member::MemberFlagIgnoreSerialization);
	END_META()

	ReferenceHandleChoice::ReferenceHandleChoice()
		: referenceHandle(CREATE_CLASS(SceneReferenceHandle))
		, chance(100)
	{ }

	MutexReferenceNode::MutexReferenceNode(const std::string& n, ECSContext* cxt)
		: ReferenceNode(n, cxt)
		, chanceIndex(-1)
	{

	}

	MutexReferenceNode::MutexReferenceNode(const std::string& n, ECSContext* cxt, const std::string& refName)
		: ReferenceNode(n, cxt, refName)
		, chanceIndex(-1)
	{

	}

	std::shared_ptr<SceneReferenceHandle>& MutexReferenceNode::getReferenceHandle()
	{ 
		if (this->choices.size() == 0)
		{
			return this->referenceHandle;
		}

		if (this->chanceIndex == -1)
		{
			this->calculateChanceIndex();
		}

		return this->choices[this->chanceIndex]->referenceHandle;
	}


	void MutexReferenceNode::calculateChanceIndex()
	{
		int32 sum = 0;
		for (const auto it : this->choices)
		{
			sum += it->chance;
		}

		int32 choice = rand() % sum;
		int32 ctr = 0;
		for (size_t i = 0; i < this->choices.size(); i++)
		{
			ctr += this->choices[i]->chance;
			if (choice < ctr)
			{
				this->chanceIndex = (int32) i;
				return;
			}
		}

		assert(false);
	}
}