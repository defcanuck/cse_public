#include "PCH.h"

#include "scripting/LuaConverter.h"

#include <luabind/operator.hpp>

namespace luabind
{

}

namespace cs
{
	void ConvertPropertySet::convertObject(const std::string& key, luabind::object& obj, cs::PropertySet& prop)
	{

		switch (luabind::type(obj))
		{
			case LUA_TTABLE:
			{
				assert(false); // bad!
			} break;

			case LUA_TNUMBER:
			{
				double num = luabind::object_cast<double>(obj);
				double intpart;
				if (std::modf(num, &intpart) == 0.0)
				{
					// integer
					cs::PropertyIntegerPtr numProp = CREATE_CLASS(cs::PropertyInteger, key, static_cast<int32>(num));
					prop.addPropertySafe(numProp);
				}
				else
				{
					// float
					cs::PropertyDoublePtr numProp = CREATE_CLASS(cs::PropertyDouble, key, static_cast<float32>(num));
					prop.addPropertySafe(numProp);
				}

			} break;
			case LUA_TSTRING:
			{
				cs::PropertyStringPtr numProp = CREATE_CLASS(cs::PropertyString, key, luabind::object_cast<std::string>(obj));
				prop.addPropertySafe(numProp);
			} break;
		}
	}

	bool ConvertPropertySet::convertTable(lua_State* L, int index, cs::PropertySet& props)
	{
		lua_pushnil(L);
		while (lua_next(L, index) != 0)
		{
			luabind::object key(luabind::from_stack(L, -2));
			std::string key_str = "";
			switch (luabind::type(key))
			{ 
				case LUA_TSTRING:
					key_str = luabind::object_cast<std::string>(key); 
					break; 
				case LUA_TNUMBER:
				{
					std::stringstream str;
					str << static_cast<int32>(luabind::object_cast<double>(key));
					key_str = str.str();
				} break;
				default:
					cs::log::info("Invalid property set key formatting");
					return false;
			}

			if (lua_istable(L, -1))
			{
				cs::PropertySetPtr tableProp = CREATE_CLASS(cs::PropertySet, key_str);
				if (convertTable(L, -2, *tableProp.get()))
				{
					props.addPropertySafe(tableProp);
				}
			}
			else
			{
				luabind::object value(luabind::from_stack(L, -1));
				convertObject(key_str, value, props);
			}

			// Pop value, keep key
			lua_pop(L, 1);
		}

		/*
		// table is in the stack at index 't'
		lua_pushnil(L);  // first key
		while (lua_next(L, index) != 0)
		{
			// uses 'key' (at index -2) and 'value' (at index -1)
			cs::log::info(luaL_typename(L, -2), " - ", luaL_typename(L, -1));

			luabind::object key(luabind::from_stack(L, -2));
			if (luabind::type(key) != LUA_TSTRING)
			{

				cs::log::info("Invalid property set formatting");
				return false;
			}

			luabind::object value(luabind::from_stack(L, -1));
			convertObject(luabind::object_cast<std::string>(key), value, props, L, index);

			// removes 'value'; keeps 'key' for next iteration
			lua_pop(L, 1);
		}
		*/

		return true;
	}
}