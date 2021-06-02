#pragma once

#include <luabind/luabind.hpp>

#include "global/Values.h"
#include "global/PropertySet.h"
#include "scripting/LuaCallback.h"

namespace cs
{

	namespace ConvertPropertySet
	{
		void convertObject(const std::string& key, luabind::object& obj, cs::PropertySet& prop);
		bool convertTable(lua_State* L, int index, cs::PropertySet& props);
	}
}

namespace luabind
{
	template<>
	struct default_converter<StringList> : native_converter_base<StringList>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
		}

		StringList from(lua_State* L, int index)
		{
			StringList list;
			for (luabind::iterator i(luabind::object(luabind::from_stack(L, index))), end; i != end; ++i)
			{
				std::string str = luabind::object_cast<std::string>(*i);
				list.push_back(str);
			}

			return list;
		}

		void to(lua_State* L, const StringList& l)
		{
			luabind::object list = luabind::newtable(L);
			for (size_t i = 0; i < l.size(); ++i)
				list[i + 1] = l[i];
			list.push(L);
		}
	};

	template <>
	struct default_converter<StringList const>
		: default_converter<StringList>
	{};

	template <>
	struct default_converter<StringList const&>
		: default_converter<StringList>
	{};


	template<>
	struct default_converter<cs::PropertySet> : native_converter_base<cs::PropertySet>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
		}

		cs::PropertySet from(lua_State* L, int index)
		{
			cs::PropertySet props("Lua");
			cs::ConvertPropertySet::convertTable(L, index, props);
			return props;
		}

		void to(lua_State* L, const cs::PropertySet& prop)
		{
			luabind::object list = luabind::newtable(L);
			toImpl(list, L, prop);
			list.push(L);
		}

		static void toImpl(luabind::object& table, lua_State* L, const cs::PropertySet& prop)
		{
			for (auto& it : prop.propVector)
			{
				switch (it->getType())
				{
					case cs::PropertyTypeString:
					{
						cs::PropertyStringPtr propString = std::static_pointer_cast<cs::PropertyString>(it);
						table[it->propName] = propString->propString;
					} break;
					case cs::PropertyTypeColor:
					{
						cs::PropertyColorPtr propColor = std::static_pointer_cast<cs::PropertyColor>(it);
						table[it->propName] = propColor->propColor;
					} break;
					case cs::PropertyTypeInteger:
					{
						cs::PropertyIntegerPtr propInt = std::static_pointer_cast<cs::PropertyInteger>(it);
						table[it->propName] = double(propInt->propNumber);
					} break;
					case cs::PropertyTypeDouble:
					{
						cs::PropertyDoublePtr propInt = std::static_pointer_cast<cs::PropertyDouble>(it);
						table[it->propName] = propInt->propNumber;
					} break;
					case cs::PropertyTypeTable:
					{
						cs::PropertySetPtr propTable = std::static_pointer_cast<cs::PropertySet>(it);
						luabind::object newTable = luabind::newtable(L);
						toImpl(newTable, L, *propTable.get());
						table[it->propName] = newTable;
					} break;
					case cs::PropertyTypeVector:
					{
						cs::PropertyVector3Ptr propInt = std::static_pointer_cast<cs::PropertyVector3>(it);
						table[it->propName] = propInt->propVector;
					} break;
					default:
						assert(false);
				}
			}
		}
	};

	template <>
	struct default_converter<cs::PropertySet const>
		: default_converter<cs::PropertySet>
	{};

	template <>
	struct default_converter<cs::PropertySet const&>
		: default_converter<cs::PropertySet>
	{};
}