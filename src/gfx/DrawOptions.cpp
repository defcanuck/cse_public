#include "PCH.h"

#include "gfx/DrawOptions.h"
#include "gfx/DrawCall.h"

namespace cs
{
	BEGIN_META_CLASS(BlendTypeBase) END_META()
	BEGIN_META_CLASS(BlendTypeZero) END_META()
	BEGIN_META_CLASS(BlendTypeOne) END_META()
	BEGIN_META_CLASS(BlendTypeSrcAlpha) END_META()
	BEGIN_META_CLASS(BlendTypeSrcAlphaInv) END_META()

	BEGIN_META_CLASS(DrawOptionsBlend)

		ADD_MEMBER(blend);
			SET_MEMBER_DEFAULT(true);

		ADD_MEMBER_PTR(srcBlendPtr);
			ADD_COMBO_META_LABEL(BlendTypeZero, "Zero");
			ADD_COMBO_META_LABEL(BlendTypeOne, "One");
			ADD_COMBO_META_LABEL(BlendTypeSrcAlpha, "Alpha");
			ADD_COMBO_META_LABEL(BlendTypeSrcAlphaInv, "One Minus Alpha");

		ADD_MEMBER_PTR(dstBlendPtr);
			ADD_COMBO_META_LABEL(BlendTypeZero, "Zero");
			ADD_COMBO_META_LABEL(BlendTypeOne, "One");
			ADD_COMBO_META_LABEL(BlendTypeSrcAlpha, "Alpha");
			ADD_COMBO_META_LABEL(BlendTypeSrcAlphaInv, "One Minus Alpha");

	END_META();

	BEGIN_META_CLASS(DepthTypeBase) END_META()
	BEGIN_META_CLASS(DepthTypeNever) END_META()
	BEGIN_META_CLASS(DepthTypeEqual) END_META()
	BEGIN_META_CLASS(DepthTypeLess) END_META()
	BEGIN_META_CLASS(DepthTypeLessEqual) END_META()
	BEGIN_META_CLASS(DepthTypeGreater) END_META()
	BEGIN_META_CLASS(DepthTypeGreaterEqual) END_META()
	BEGIN_META_CLASS(DepthTypeNotEqual) END_META()
	BEGIN_META_CLASS(DepthTypeAlways) END_META()

	BEGIN_META_CLASS(DrawOptionsDepth)
		ADD_MEMBER(depthTest);
			SET_MEMBER_DEFAULT(true);
		ADD_MEMBER(depthWrite);
			SET_MEMBER_DEFAULT(true);

		ADD_MEMBER_PTR(depthFuncPtr);
            // MetaData* m = MetaCreator<DepthTypeNever>::get();
            // memberPtr->addComboMeta<DepthTypeNever>(m, "label", "meta");
    
			ADD_COMBO_META_LABEL(DepthTypeNever, "Never");
			ADD_COMBO_META_LABEL(DepthTypeEqual, "Equal");
			ADD_COMBO_META_LABEL(DepthTypeLess, "Less");
			ADD_COMBO_META_LABEL(DepthTypeLessEqual, "Less Equal");
			ADD_COMBO_META_LABEL(DepthTypeGreater, "Greater");
			ADD_COMBO_META_LABEL(DepthTypeGreaterEqual, "Greater Equal");
			ADD_COMBO_META_LABEL(DepthTypeNotEqual, "Not Equal");
			ADD_COMBO_META_LABEL(DepthTypeAlways, "Always");
			
	END_META();

	BEGIN_META_CLASS(DrawOptions)
		ADD_MEMBER(depth);			
		ADD_MEMBER(blend);
	END_META();

	void DrawOptionsDepth::populate(std::shared_ptr<DrawCall>& draw) const
	{
		draw->depthFunc = this->depthFuncPtr->getType();
		draw->depthWrite = this->depthWrite;
		draw->depthTest = this->depthTest;
	}

	void DrawOptionsBlend::populate(std::shared_ptr<DrawCall>& draw) const
	{
		draw->blend = this->blend;
		draw->srcBlend = this->srcBlendPtr->getType();
		draw->dstBlend = this->dstBlendPtr->getType();
	}

	void DrawOptions::populate(std::shared_ptr<DrawCall>& draw) const
	{
		this->populateBlend(draw);
		this->populateDepth(draw);
	}

	void DrawOptions::populateDepth(std::shared_ptr<DrawCall>& draw) const
	{
		this->depth.populate(draw);
	} 

	void DrawOptions::populateBlend(std::shared_ptr<DrawCall>& draw) const
	{
		this->blend.populate(draw);
	}

}
