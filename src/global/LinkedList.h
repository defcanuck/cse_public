#pragma once

#include <memory>


namespace cs
{
	template<class T>
	class SharedNode
	{
	public:

		typedef std::shared_ptr<SharedNode<T>> SharedNodePtr;

		SharedNode()
			: value(T())
			, next(nullptr) { }

		SharedNode(const T& val, SharedNodePtr ptr_next = NULL)
			: value(val)
			, next(ptr_next) { }

		T* operator->() 
		{ 
			return &this->object; 
		}
		
		T value;
		SharedNodePtr next;
	};

	template <class T>
	class SharedLinkedList
	{

		typedef std::shared_ptr<SharedNode<T>> SharedNodePtr;
		typedef bool(*sortFunc)(const T&, const T&);
	public:

		SharedLinkedList()
		{
			this->head = nullptr;
			this->tail = nullptr;
			this->numNodes = 0;
		}

		~SharedLinkedList()
		{
			this->empty();
		}

		SharedNodePtr push_back(const T& value)
		{
			SharedNodePtr ptr = std::make_shared<SharedNode<T>>(value);
			this->push_back_impl(ptr);
			return ptr;
		}

		SharedNodePtr push_back(const SharedNodePtr& ptr)
		{
			this->push_back_impl(ptr);
			// this->sanity();
			return ptr;
		}

		void sanity()
		{
			/*
			size_t counter = 0;
			SharedNodePtr traverse_ptr = this->head;
			while (traverse_ptr.get())
			{
				counter++;
				traverse_ptr = traverse_ptr->next;
			}
			assert(counter == this->numNodes);
			*/
		}

		SharedNodePtr push_sort(const T& value, sortFunc func)
		{
			SharedNodePtr traverse_ptr = this->head;
			SharedNodePtr prev_ptr = nullptr;

			if (!traverse_ptr.get())
			{
				return this->push_back(value);
			}

			while (traverse_ptr.get())
			{
				if ((*func)(value, traverse_ptr->value))
				{
					SharedNodePtr ptr = std::make_shared<SharedNode<T>>(value);
					this->insert_inpl(ptr, traverse_ptr, prev_ptr);
					// this->sanity();
					return ptr;
				}
				prev_ptr = traverse_ptr;
				traverse_ptr = traverse_ptr->next;
			}

			return this->push_back(value);
		}

		const size_t getSize() const { return this->numNodes; }

		void traverse(void(*traverseFunc)(const T&, void*), void* data)
		{
			SharedNodePtr current = this->head;
			size_t counter = 0;
			while (current != nullptr)
			{
				traverseFunc(current->value, data);
				counter++;
				current = current->next;
			}
			assert(counter == numNodes);
		}

		void empty()
		{
			SharedNodePtr cur = this->head; 
			SharedNodePtr nxt = nullptr;
			while (cur)
			{
				cur->next = nxt;
				cur->next.reset();
				cur = nxt;
			}
			this->numNodes = 0;
		}

		SharedNodePtr head;
		SharedNodePtr tail;
		size_t numNodes;

	private:

		inline void push_back_impl(const SharedNodePtr& ptr)
		{
			if (!this->head)
			{
				this->head = ptr;
				this->tail = ptr;
				this->numNodes++;
				return;
			}

			this->tail->next = ptr;
			this->tail = ptr;
			this->numNodes++;
		}

		inline void insert_inpl(SharedNodePtr& ptr, SharedNodePtr& at, SharedNodePtr& prev)
		{
			if (!this->head)
			{
				this->head = ptr;
				this->tail = ptr;
				this->numNodes++;
				return;
			}

			// we're the tail - yay
			if (!at.get())
			{
				this->push_back_impl(ptr);
				return;
			}

			ptr->next = at;
			if (prev.get())
			{
				assert(prev->next.get() == at.get()); // sanity
				prev->next = ptr;
			}

			if (at.get() == this->head.get())
			{
				this->head = ptr;
			}
			this->numNodes++;
		}
	};
}