#pragma once

#include "ClassDef.h"

#include "gfx/Types.h"

namespace cs
{
	class DrawCall;

	CLASS_DEFINITION_REFLECT(BlendTypeBase)
	public:
		virtual BlendType getType() { return BlendNone; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(BlendTypeZero, BlendTypeBase)
	public:
		virtual BlendType getType() { return BlendZero; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(BlendTypeOne, BlendTypeBase)
	public:
		virtual BlendType getType() { return BlendOne; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(BlendTypeSrcAlpha, BlendTypeBase)
	public:
		virtual BlendType getType() { return BlendSrcAlpha; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(BlendTypeSrcAlphaInv, BlendTypeBase)
	public:
		virtual BlendType getType() { return BlendOneMinusSrcAlpha; }
	};

	inline BlendTypeBasePtr createGUIForBlendType(BlendType type)
	{
		switch (type)
		{
			case BlendZero: return CREATE_CLASS(BlendTypeZero);
			case BlendOne: return CREATE_CLASS(BlendTypeOne);
			case BlendSrcAlpha: return CREATE_CLASS(BlendTypeSrcAlpha);
			case BlendOneMinusSrcAlpha: return CREATE_CLASS(BlendTypeSrcAlphaInv);
		}
		return BlendTypeBasePtr();
	}


	CLASS_DEFINITION_REFLECT(DrawOptionsBlend)
	public:
		DrawOptionsBlend()
			: blend(true)
			, srcBlendPtr(CREATE_CLASS_CAST(BlendTypeBase, BlendTypeSrcAlpha))
			, dstBlendPtr(CREATE_CLASS_CAST(BlendTypeBase, BlendTypeSrcAlphaInv))
		{ }

		DrawOptionsBlend(const DrawOptionsBlend& rhs)
			: blend(rhs.blend)
			, srcBlendPtr(rhs.srcBlendPtr)
			, dstBlendPtr(rhs.dstBlendPtr)
		{ }

		void operator=(const DrawOptionsBlend& rhs)
		{
			this->blend = rhs.blend;
			this->srcBlendPtr = rhs.srcBlendPtr;
			this->dstBlendPtr = rhs.dstBlendPtr;
		}

		bool operator!=(const DrawOptionsBlend& rhs) const
		{
			return this->blend != rhs.blend || 
				this->srcBlendPtr->getType() != rhs.srcBlendPtr->getType() ||
				this->dstBlendPtr->getType() != rhs.dstBlendPtr->getType();
		}

		bool operator==(const DrawOptionsBlend& rhs) const
		{
			return this->blend == rhs.blend && 
				this->srcBlendPtr->getType() == rhs.srcBlendPtr->getType() && 
				this->dstBlendPtr->getType() == rhs.dstBlendPtr->getType();
		}

		void setSourceBlend(BlendType type) { this->srcBlendPtr = createGUIForBlendType(type); }
		void setDestBlend(BlendType type) { this->dstBlendPtr = createGUIForBlendType(type); }

		void populate(std::shared_ptr<DrawCall>& draw) const;

		bool getBlendEnabled() const { return this->blend; }
		BlendType getSourceBlend() const { return this->srcBlendPtr->getType(); }
		BlendType getDestBlend() const { return this->dstBlendPtr->getType(); }

	private:

		bool blend;
		BlendTypeBasePtr srcBlendPtr;
		BlendTypeBasePtr dstBlendPtr;
	};

	CLASS_DEFINITION_REFLECT(DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthNone; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeNever, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthNever; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeEqual, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthEqual; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeLess, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthLess; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeLessEqual, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthLessEqual; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeGreater, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthGreater; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeGreaterEqual, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthGreaterEqual; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeNotEqual, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthNotEqual; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(DepthTypeAlways, DepthTypeBase)
	public:
		virtual DepthType getType() { return DepthAlways; }
	};

	inline DepthTypeBasePtr createGUIForDepthType(DepthType type)
	{
		switch (type)
		{
			case DepthNever: return CREATE_CLASS(DepthTypeNever);
			case DepthEqual: return CREATE_CLASS(DepthTypeEqual);
			case DepthLess: return CREATE_CLASS(DepthTypeLess);
			case DepthLessEqual: return CREATE_CLASS(DepthTypeLessEqual);
			case DepthGreater: return CREATE_CLASS(DepthTypeGreater);
			case DepthGreaterEqual: return CREATE_CLASS(DepthTypeGreaterEqual);
			case DepthNotEqual: return CREATE_CLASS(DepthTypeNotEqual);
			case DepthAlways: return CREATE_CLASS(DepthTypeAlways);
		}
		return DepthTypeBasePtr();
	}

	CLASS_DEFINITION_REFLECT(DrawOptionsDepth)
	public:
		DrawOptionsDepth()
			: depthTest(true)
			, depthWrite(true)
			, depthFuncPtr(CREATE_CLASS(DepthTypeLess))
		{ }

		DrawOptionsDepth(const DrawOptionsDepth& rhs)
			: depthTest(rhs.depthTest)
			, depthWrite(rhs.depthWrite)
			, depthFuncPtr(rhs.depthFuncPtr)

		{ }

		void operator=(const DrawOptionsDepth& rhs)
		{
			this->depthTest = rhs.depthTest;
			this->depthWrite = rhs.depthWrite;
			this->depthFuncPtr = rhs.depthFuncPtr;
		}

		bool operator==(const DrawOptionsDepth& rhs) const
		{
			return this->depthTest == rhs.depthTest && 
				this->depthWrite == rhs.depthWrite && 
				this->depthFuncPtr->getType() == rhs.depthFuncPtr->getType();
		}

		void populate(std::shared_ptr<DrawCall>& draw) const;

		void setDepthTest(bool dt) { this->depthTest = dt; }
		void setDepthWrite(bool dw) { this->depthWrite = dw; }
		void setDepthType(DepthType type)
		{
			this->depthFuncPtr = createGUIForDepthType(type);
		}

	private:

		bool depthTest;
		bool depthWrite;
		
		DepthTypeBasePtr depthFuncPtr;
	};

	CLASS_DEFINITION_REFLECT(DrawOptions)
	public:
		DrawOptions()
			: depth(DrawOptionsDepth())
			, blend(DrawOptionsBlend())
		{ }

		void operator=(const DrawOptions& rhs)
		{
			this->depth = rhs.depth;
			this->blend = rhs.blend;
		}

		bool operator==(const DrawOptions& rhs)
		{
			return this->depth == rhs.depth && this->blend == rhs.blend;
		}

		void populate(std::shared_ptr<DrawCall>& draw) const;
		void populateDepth(std::shared_ptr<DrawCall>& draw) const;
		void populateBlend(std::shared_ptr<DrawCall>& draw) const;

		DrawOptionsDepth depth;
		DrawOptionsBlend blend;
	};
}