#include "PCH.h"

#include "os/InputManager.h"
#include "os/LogManager.h"

namespace cs
{

	const float32 kSwipeDistance = 0.2f;

	struct InputClickListenerPrioritySort
	{
		bool operator()(const InputManager::InputClickListenerPriority& a, const InputManager::InputClickListenerPriority& b)
		{
			return a.second.priority > b.second.priority;
		}
	};

	struct InputKeyListenerPrioritySort
	{
		bool operator()(const InputManager::InputKeyListenerPriority& a, const InputManager::InputKeyListenerPriority& b)
		{
			return a.second.priority > b.second.priority;
		}
	};

	InputManager::InputManager()
	{
		this->activeInput = nullptr;
		this->enabled = true;
		this->isButtonPressed = false;

		for (int i = 0; i < ClickMAX; i++)
		{
			states[(ClickInput)i] = ClickParams();
		}
	}

	InputManager::~InputManager()
	{
	}

	void InputManager::onInputDown(ClickInput input, const vec2& position)
	{
		if (!this->enabled)
			return;

		ClickParams& params = states[input];
		params.position = position;
		params.lastPosition = position;
		params.velocity = vec2(0.0f, 0.0f);

		if (params.state == TouchNone)
		{
			params.state = TouchPressed;
			params.startPosition = position;
			this->triggerNotification(input);
		}

		params.state = TouchDown;
		this->triggerNotification(input);
		this->isButtonPressed = true;
	}

	void InputManager::onInputUp(ClickInput input, bool forceRelease)
	{
		if (!this->enabled && !this->isButtonPressed)
			return;

		this->isButtonPressed = false;
		ClickParams& params = states[input];
		if (params.state != TouchMove)
		{

			params.state = TouchReleased;
			this->triggerNotification(input);
		}
		else
		{
			float32 dist = glm::distance(params.position, params.startPosition);
			if (dist > kSwipeDistance)
			{
				float32 xAmt = params.position.x - params.startPosition.x;
				float32 yAmt = params.position.y - params.startPosition.y;
				float32 xAbsAmt = abs(xAmt);
				float32 yAbsAmt = abs(yAmt);
				TouchState swipeState = TouchNone;

				if (xAbsAmt > (yAbsAmt * 1.5))
				{
					if (xAmt < 0)
						swipeState = TouchSwipeLeft;
					else
						swipeState = TouchSwipeRight;
				}
				if (yAbsAmt > (xAbsAmt * 1.5))
				{
					if (yAmt < 0)
						swipeState = TouchSwipeDown;
					else
						swipeState = TouchSwipeUp;
				}

				if (swipeState != TouchNone)
				{
					switch (swipeState)
					{
						case TouchSwipeLeft: log::info("Swipe Left"); break;
						case TouchSwipeRight: log::info("Swipe Right"); break;
						case TouchSwipeDown: log::info("Swipe Down"); break;
						case TouchSwipeUp: log::info("Swipe Up"); break;
					}
					params.state = swipeState;
					this->triggerNotification(input);
				}
               
               
                params.state = TouchNone;
                return;
			}
            else if (forceRelease)
            {
                params.state = TouchReleased;
                this->triggerNotification(input);
            }
		}

		params.state = TouchUp;
		this->triggerNotification(input);

		params.state = TouchNone;
	}

	void InputManager::onInputMove(ClickInput input, const vec2& position)
	{
		if (!this->enabled)
			return;

		ClickParams& params = states[input];
		params.state = TouchMove;
		params.lastPosition = params.position;
		params.position = position;
		params.velocity = params.lastPosition - params.position;

		this->triggerNotification(input);
	}

	void InputManager::onInputZoom(ClickInput input, float32 zoom)
	{
		if (!this->enabled)
			return;

		ClickParams& params = states[input];
		params.state = TouchWheel;
		params.zoom = zoom;

		this->triggerNotification(input);
	}

	void InputManager::updateClickPriorityList()
	{
		this->clickPriorityList.clear();
		for (auto it : this->clickListeners)
			this->clickPriorityList.push_back(it.second);

		std::sort(this->clickPriorityList.begin(), this->clickPriorityList.end(), InputClickListenerPrioritySort());
	}

	void InputManager::updateKeyPriorityList()
	{
		this->keyPriorityList.clear();
		for (auto it : this->keyListeners)
			this->keyPriorityList.push_back(it.second);

		std::sort(this->keyPriorityList.begin(), this->keyPriorityList.end(), InputKeyListenerPrioritySort());
	}

	void InputManager::triggerNotification(ClickInput input)
	{
		if (!this->enabled)
		{
			log::info("Input disabled - ignoring notifications!");
			return;
		}

		for (auto it : this->clickPriorityList)
		{
			ClickParams& params = states[input];
			if (it.first(input, params))
			{
				//if (input == ClickPrimary && (params.state == TouchDown || params.state == TouchPressed))
				//	log_info("Consumed listener: ", it.second.tag);

				break;
			}
		}
	}

	ClickParams& InputManager::getClickState(ClickInput input)
	{
		return states[input];
	}

	void InputManager::onKeyDown(uint32 key, uint32 flags)
	{
		if (!this->enabled)
			return;

		for (auto it : this->keyPriorityList)
		{
			if (it.first(key, flags, true))
			{
				break;
			}
		}
	}

	void InputManager::onKeyUp(uint32 key, uint32 flags)
	{
		if (!this->enabled)
			return;

		for (auto it : this->keyPriorityList)
		{
			if (it.first(key, flags, false))
			{
				break;
			}
		}
	}
}
