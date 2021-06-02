#pragma once

#include "gfx/Attribute.h"

namespace cs
{
	struct VertexDeclaration
	{
		VertexDeclaration();
		VertexDeclaration(AttributeMap& attribs);
		VertexDeclaration(const VertexDeclaration& decl)
            : attributes(decl.attributes)
            , attribList(decl.attribList)
            , stride(decl.stride)
        { }
        
		uint32 getStride() const { return stride; }
        uint32 getNumAttributes() const { return uint32(this->attribList.size()); }
        const Attribute* getAttribData() const { return &this->attribList[0]; }
        const Attribute* getAttribAt(size_t i) const { return &this->attribList[i]; }
        
		uint32 getMask() const
		{
			uint32 mask = 0;
			for (auto& it : this->attributes)
				mask += 0x1 << uint32(it.first);
			return mask;
		}

		uint32 addAttrib(AttributeType type, Attribute attrib)
		{
			this->attributes[type] = attrib;
            this->attribList.push_back(attrib);
			this->calcStride();
			return this->stride;
		}
        
        void removeAttrib(AttributeType type)
        {
            AttributeMap::iterator it = this->attributes.find(type);
            if (it != this->attributes.end())
            {
                this->attributes.erase(it);
                for (size_t i = 0; i < this->attribList.size(); ++i)
                {
                    if (this->attribList[i].type == type)
                    {
                        this->attribList.erase(this->attribList.begin() + i);
                        return; 
                    }
                }
            }
        }

		void getAttributeList(AttributeList& list)
		{
			for (auto& it : this->attributes)
				list.push_back(it.second);
		}

		const Attribute* getAttrib(AttributeType type) 
		{
			if (this->attributes.find(type) == this->attributes.end())
				return nullptr;

			return &this->attributes[type];
		}

		template <typename T>
		T* getAttributePointer(uchar* data, AttributeType type)
		{
			if (this->attributes.find(type) == this->attributes.end())
				return nullptr;
			return reinterpret_cast<T*>(((uchar*) data) + this->attributes[type].offset);
		}

		template <typename T>
		T* getAttributePointerAtIndex(uchar* data, AttributeType type, uint32 index)
		{
			if (this->attributes.find(type) == this->attributes.end())
				return nullptr;
			uint32 byteOffset = (this->stride * index) + this->attributes[type].offset;
			return reinterpret_cast<T*>(((uchar*) data) + byteOffset);
		}

	private:

		AttributeMap attributes;
        AttributeList attribList;
		uint32 stride;

		void calcStride() 
		{
			this->stride = 0;
			for (auto& it : this->attributes)
				this->stride += it.second.count * kTypeSize[it.second.dataType];
		}
	};

    
    struct VertexDeclarationCompare
    {
       bool operator()(const VertexDeclaration& lhs, const VertexDeclaration& rhs) const
       {
           
           if (lhs.getNumAttributes() < rhs.getNumAttributes()) return true;
           if (lhs.getNumAttributes() > rhs.getNumAttributes()) return false;
           
           if (lhs.getStride() < rhs.getStride()) return true;
           if (lhs.getStride() > rhs.getStride()) return false;
           return false;
       }
    };
}
