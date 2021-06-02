#include "PCH.h"

#include "ui/UIStack.h"
#include "gfx/Renderer.h"

namespace cs
{
	UIStack::~UIStack()
	{
		this->popAllDocuments();
	}

	void UIStack::draw(const RectI& screenSize, UIBatchPass pass)
	{
        UniformPtr viewportValue = SharedUniform::getInstance().getUniform("viewport");
        viewportValue->setValue(vec2((float32) screenSize.size.w, (float32) screenSize.size.h));
        
		RenderInterface* render_interface = RenderInterface::getInstance();
		for (auto it : this->sortedStack)
		{
			if (it->getPendingRemove())
				continue;

			it->draw(screenSize, pass);
		}

		if (this->isOverlay && this->overlay)
		{
			this->overlay->draw(screenSize, pass);
		}
	}

	void UIStack::updateStack()
	{
		if (this->toRemove.size() > 0)
		{
			this->actionElement = nullptr;
		}

		for (auto it : this->toRemove)
		{
			for (UIDocumentStack::iterator doc = this->stack.begin(); doc != this->stack.end(); doc++)
			{
				if (doc->get() == it.get())
				{
					it->setPendingRemove(false);
					this->stack.erase(doc);
					break;
				}
			}
		}
		this->toRemove.clear();
	}
	
	void UIStack::process(float32 dt, RectI& screenSize)
	{
		this->updateStack();

		struct 
		{
			bool operator()(const UIDocumentPtr &a, const UIDocumentPtr &b) 
			{
				return a->getSortOrder() < b->getSortOrder();
			}
		} UIDocumentStackSort;

		this->sortedStack = this->stack;
		std::sort(this->sortedStack.begin(), this->sortedStack.end(), UIDocumentStackSort);

		float32 doc_depth = this->depthOffset;

		if (this->sortedStack.size() > 0)
		{
			for (int32 pass = 0; pass < UIBatchPassMAX; ++pass)
			{
				if (this->passMask.test((UIBatchPass)pass))
				{
					for (auto& it : this->sortedStack)
					{
						if (it->getPendingRemove())
							continue;

						it->process(dt, doc_depth, screenSize, (UIBatchPass)pass);
					}

					if (this->isOverlay && this->overlay)
					{
						this->overlay->process(dt, doc_depth, screenSize, (UIBatchPass)pass);
					}
				}
			}
		}
	}
	

	UIDocumentPtr UIStack::getTop()
	{
		if (this->stack.size() > 0)
		{
			return this->stack.front();
		}

		return UIDocumentPtr();
	}

	UIDocumentPtr UIStack::getDocumentByName(const std::string& docName)
	{
		for (auto& it : this->stack)
		{
			if (it->getName() == docName)
				return it;
		}
		return UIDocumentPtr();
	}

	void UIStack::pushDocument(UIDocumentPtr& doc)
	{
		this->pushDocumentImpl(doc, true);
	}

	void UIStack::pushDocument(UIDocumentPtr& doc, bool inheritStackSize)
	{
		this->pushDocumentImpl(doc, inheritStackSize);
	}

	void UIStack::pushDocumentImpl(UIDocumentPtr& doc, bool inheritStackSize)
	{
		this->stack.push_front(doc);

		if (inheritStackSize)
			doc->init(size);
	}


	void UIStack::removeDocument(const std::string& documentName)
	{
		for (UIDocumentStack::iterator doc = this->stack.begin(); doc != this->stack.end(); doc++)
		{
			if ((*doc)->getName() == documentName)
			{
				(*doc)->setPendingRemove(true);
				this->toRemove.push_back((*doc));
				break;
			}
		}
	}

	void UIStack::removeDocument(UIDocumentPtr& doc)
	{
		toRemove.push_back(doc);
	}

	void UIStack::popDocument()
	{
		UIDocumentPtr& doc = this->stack.back();
		this->toRemove.push_back(doc);
	}

	void UIStack::popUnlockedDocuments()
	{
		UIDocumentStack::iterator it = this->stack.begin();
		while (it != this->stack.end())
		{
			if (!(*it)->getLocked())
				it = this->stack.erase(it);
			else
				++it;
		}
	}

	void UIStack::popAllDocuments()
	{
		for (auto& it : this->stack)
		{
			this->toRemove.push_back(it);
		}
	}

	void UIStack::clearDocuments()
	{
		this->stack.clear();
		this->sortedStack.clear();
	}

	bool UIStack::onCursor(ClickInput input, ClickParams& params)
	{
		bool ret = false;
		if (this->actionElement)
		{
			if (params.state == TouchMove)
			{
				if (!this->actionElement->onUpdate(params.position))
				{
					this->actionElement->onExit(input);
					this->actionElement = nullptr;
				}

				this->onCursorMove(params.position);

			} else if (params.state == TouchReleased || params.state == TouchUp) {

				this->actionElement->onExit(input);
				this->actionElement = nullptr;

				// also bubble up the release state to other documents (ie. Lua specifically)
				this->onCursorStackImpl(input, params);
			}
			
			return true;

		} 
		else 
		{
			ret = this->onCursorStackImpl(input, params);
		}
		return ret;
	}

	void UIStack::onCursorMove(const vec2& pos)
	{
		UIDocumentStack curStack = this->stack;
		for (auto it : curStack)
		{
			it->onCursorMove(pos);
		}
	}

	bool UIStack::onCursorStackImpl(ClickInput input, ClickParams& params)
	{
		bool ret = false;
		UIClickResults results;
		UIDocumentStack curStack = this->stack;

		for (auto it : curStack)
		{
			bool ret = it->onCursor(input, params, results);

			for (auto itt : results)
			{
				UIElement* element = itt.first;
				UIElementClick& click = itt.second;

				// returns true if the element was clicked
				if (element->onEnter(click))
				{
					if (!this->actionElement)
					{
						this->actionElement = itt.first;
					}

					return true;
				}
			}

			// Document consumed the click
			if (ret)
			{
				return true;
			}
		}

		return ret;
	}
}
