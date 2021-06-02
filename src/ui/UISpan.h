#pragma once

#include "math/Rect.h"

namespace cs
{
	enum SpanMeasure
	{
		SpanNone = -1,
		SpanPixels,
		SpanPercent,
		SpanPoints,
	};

	enum HAlign
	{
		HAlignNone = -1,
		HAlignLeft,
		HAlignRight,
		HAlignCenter,
		HAlignMAX
	};

	enum VAlign
	{
		VAlignNone = -1,
		VAlignBottom,
		VAlignTop,
		VAlignCenter,
		VAlignMAX
	};

	template <typename T, typename C>
	void evaluatePosition(T align, const C& spanWidth, const C& elemWidth, C& result, const C& padding)
	{
		switch (align)
		{
		case T(0):
			result = C() + padding;
			break;
		case T(1):
			result = spanWidth - elemWidth - padding;
			break;
		case T(2):
			result = C(float32(spanWidth * 0.5f) - float32(elemWidth * 0.5f));
			break;
		}
	}

	struct UIDimension
	{
		UIDimension() :
			measure(SpanPixels),
			value(0.0f) { }

		static float32 getContentScale();

		SpanMeasure measure;
		float32 value;

		float32 evaluate(float32 parent)
		{
			switch (this->measure)
			{
                case SpanPixels:
                    return value;
                case SpanPercent:
                    return (value * 0.01f) * parent;
				case SpanPoints:
					return value * UIDimension::getContentScale();
                default: break;
                    
			}
			return 0.0f;
		}
	};

	template <typename T>
	struct UIPosition
	{
		UIPosition() :
			alignment(T(0)) { }

		UIDimension value;
		T alignment;

		float32 evaluate(float32 elem_dimm, float32 parent_pos, float32 parent_dimm)
		{
			float real_value = value.evaluate(parent_dimm);
			switch (alignment)
			{
				case T(0):
					return real_value + parent_pos;
				case T(1):
				{
					float32 parentTop = parent_pos + parent_dimm;
					return parentTop - (elem_dimm) + real_value;
				}
				case T(2):
				{
					float32 parentCenter = parent_pos + parent_dimm * 0.5f;
					return parentCenter - (elem_dimm * 0.5f) + real_value;
				}
                default: break;
			}
			return 0.0f;
		}
	};

	typedef UIPosition<HAlign> UIPositionHorizontal;
	typedef UIPosition<VAlign> UIPositionVertical;

	struct UISpan
	{
		UISpan() :
			rawWidth(UIDimension()),
			rawHeight(UIDimension()),
			rawX(UIPositionHorizontal()),
			rawY(UIPositionVertical()) { }

		inline RectF evaluateToParent(const RectF& parent, bool hintWidth = false, bool hintHeight = false, float32 textureRatio = 0.0)
		{
			assert(!(hintWidth && hintHeight));

			float32 xa = 0.0f, ya = 0.0f;
			float32 xw = 0.0f, yh = 0.0f;
			
			xw = rawWidth.evaluate(parent.size.w);
			yh = rawHeight.evaluate(parent.size.h);
			
			if (hintWidth)
			{
				xw = yh * textureRatio;
			}
			if (hintHeight)
			{
				yh = xw * 1.0f / textureRatio;
			}

			xa = rawX.evaluate(xw, parent.pos.x, parent.size.w);
			ya = rawY.evaluate(yh, parent.pos.y, parent.size.h);
			 
			return RectF(xa, ya, xw, yh);
		}

		inline PointF evaluateDimensions(const PointF& parent)
		{
			float32 w = rawWidth.evaluate(parent.w);
			float32 h = rawHeight.evaluate(parent.h);

			return PointF(w, h);
		}

		const RectF getRawRect() const
		{
			return RectF(
				this->rawX.value.value,
				this->rawY.value.value,
				this->rawWidth.value,
				this->rawHeight.value);
		}

		void setMeasureDimensions(RectF& measureDimm, SpanMeasure hm = SpanPixels, SpanMeasure vm = SpanPixels)
		{
			this->rawWidth.measure = hm;
			this->rawHeight.measure = vm;
			this->rawWidth.value = measureDimm.size.w;
			this->rawHeight.value = measureDimm.size.h;
		}

		void setWidth(float32 w)
		{
			this->rawWidth.value = w;
		}

		void setWidth(float32 w, SpanMeasure hm)
		{
			this->rawWidth.value = w;
			this->rawWidth.measure = hm;
		}

		void setHeight(float32 h)
		{
			this->rawHeight.value = h;
		}

		void setHeight(float32 h, SpanMeasure vm)
		{
			this->rawHeight.value = h;
			this->rawHeight.measure = vm;
		}

		void setPosition(vec2 pos, HAlign halign = HAlignNone, VAlign valign = VAlignNone, SpanMeasure measureX = SpanNone, SpanMeasure measureY = SpanNone)
		{
			this->setX(pos.x, halign, measureX);
			this->setY(pos.y, valign, measureY);
		}

		void setX(float32 x, SpanMeasure measure = SpanNone)
		{
			this->rawX.value.value = x;
			if (measure != SpanNone)
				this->rawX.value.measure = measure;
		}

		void setX(float32 x, HAlign align = HAlignNone, SpanMeasure measure = SpanNone)
		{
			this->rawX.value.value = x;
			if (align != HAlignNone)
				this->rawX.alignment = align;
			if (measure != SpanNone)
				this->rawX.value.measure = measure;
		}

		void setY(float32 y, VAlign align = VAlignNone, SpanMeasure measure = SpanNone)
		{
			this->rawY.value.value = y;
			if (align != VAlignNone)
				this->rawY.alignment = align;
			if (measure != SpanNone)
				this->rawY.value.measure = measure;
		}

		void setY(float32 y, SpanMeasure measure = SpanNone)
		{
			this->rawY.value.value = y;
			if (measure != SpanNone)
				this->rawY.value.measure = measure;
		}

		const UIPositionHorizontal& getRawX() const { return this->rawX; }
		const UIPositionVertical& getRawY() const { return this->rawY; }

		const UIDimension& getRawWidth() const { return this->rawWidth; }
		const UIDimension& getRawHeight() const { return this->rawHeight; }

	private:

		UIPositionHorizontal rawX;
		UIPositionVertical rawY;
		UIDimension rawWidth;
		UIDimension rawHeight;

	};
}