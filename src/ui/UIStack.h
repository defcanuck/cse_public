#pragma once

#include "ClassDef.h"
#include "ui/UIDocument.h"
#include "global/Singleton.h"
#include "math/Rect.h"

#include "os/InputManager.h"
#include "gfx/RenderTexture.h"
#include "gfx/PostProcess.h"

#include <deque>

namespace cs
{
	CLASS_DEFINITION(UIStack)
	public:

		UIStack()
			: actionElement(nullptr)
			, depthOffset(0.0f)
            , isOverlay(true)
		{ 
			this->passMask.set(UIBatchPassMain);
		}

		virtual ~UIStack();

		void pushDocument(UIDocumentPtr& doc);
		void pushDocument(UIDocumentPtr& doc, bool inheritStackSize);
		void popDocument();
		void popAllDocuments();
		void popUnlockedDocuments();
		void clearDocuments();

		UIDocumentPtr getTop();
		UIDocumentPtr getDocumentByName(const std::string& docName); // SLOW!

		void removeDocument(const std::string& documentName);
		void removeDocument(UIDocumentPtr& doc);
		void updateStack();

		void draw(const RectI& screenSize, UIBatchPass pass = UIBatchPassMain);
		void process(float32 dt, RectI& screenSize);

		bool onCursor(ClickInput input, ClickParams& params);
		void onCursorMove(const vec2& pos);

		void setSize(RectI& size) { this->size = size; }
		void setDepthOffset(float32 offset) { this->depthOffset = offset; }
		void clearActionElement() { this->actionElement = nullptr; }

		void setRenderOnPass(UIBatchPass pass) { this->passMask.set(pass); }
		bool isPassSet(UIBatchPass pass) { return this->passMask.test(pass); }

		void setOverlay(UIDocumentPtr& doc) { this->overlay = doc; }
        void toggleOverlay(bool isOn) { this->isOverlay = isOn; }

	private:

		void pushDocumentImpl(UIDocumentPtr& doc, bool inheritStackSize);
		bool onCursorStackImpl(ClickInput input, ClickParams& params);

		typedef std::deque<UIDocumentPtr> UIDocumentStack;
		typedef std::vector<UIDocumentPtr> UIDocumentList;
		UIDocumentStack stack;
		UIDocumentStack sortedStack;

		UIDocumentList toRemove;

		UIElement* actionElement;
		RectI size;
		float32 depthOffset;

		UIBatchPassMask passMask;

		UIDocumentPtr overlay;
        bool isOverlay;

	};
	
}
