#include "PCH.h"

#include "ecs/comp/SerializeComponent.h"

namespace cs
{
	//MetaCreator<RemQual<TestComponent>::type> 
	//	asdf1234("TestComponent", sizeof(TestComponent));

	BEGIN_META_CLASS(TestComponent)
		ADD_MEMBER(testvalue);
	END_META()

	void TestComponent::compileMe()
	{
		std::cout << "HERE I AM!" << std::endl;
	}
}