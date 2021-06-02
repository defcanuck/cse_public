#pragma once

#include "ClassDef.h"

#include <string>

namespace cs
{
	CLASS_DEFINITION(InputFilter)
	
	public:
		virtual std::string filter(std::string& str)
		{
			return str;
		}

		virtual void set(const std::string& str)
		{
			log::print(LogError, "No setter defined!: ", str);
		}

		virtual std::string get()
		{
			return "";
		}
	};

CLASS_DEFINITION_DERIVED(FloatInputFilter, InputFilter)

	public:

		FloatInputFilter(float32* fval) : float_value(fval) { }

		virtual std::string filter(std::string& str)
		{
			std::string adj_str;
			for (size_t c = 0; c < str.length(); c++)
			{
				if ((str[c] >= '0' && str[c] <= '9') || 
					str[c] == '.' || 
					str[c] == '-')
					adj_str = adj_str + str[c];
			}
			return adj_str;
		}

		virtual void set(const std::string& str)
		{
			*this->float_value = (float32) atof(str.c_str());
		}
		
		virtual std::string get()
		{
			std::stringstream str;
			str.precision(2);
			str << std::fixed << *this->float_value;
			return str.str();
		}

	private:

		float32* float_value;
	};

	CLASS_DEFINITION_DERIVED(IntInputFilter, InputFilter)

	public:

		IntInputFilter(int32* ival) : int_value(ival) { }

		virtual std::string filter(std::string& str)
		{
			std::string adj_str;
			for (size_t c = 0; c < str.length(); c++)
			{
				if ((str[c] >= '0' && str[c] <= '9') ||
					str[c] == '-')
					adj_str = adj_str + str[c];
			}
			return adj_str;
		}

		virtual void set(const std::string& str)
		{
			*this->int_value = (int32) atoi(str.c_str());
		}

		virtual std::string get()
		{
			std::stringstream str;
			str << *this->int_value;
			return str.str();
		}

	private:

		int32* int_value;
	};

	CLASS_DEFINITION_DERIVED(UnsignedIntInputFilter, InputFilter)

	public:

		UnsignedIntInputFilter(uint32* ival) : uint_value(ival) { }

		virtual std::string filter(std::string& str)
		{
			std::string adj_str;
			for (size_t c = 0; c < str.length(); c++)
			{
				if ((str[c] >= '0' && str[c] <= '9'))
					adj_str = adj_str + str[c];
			}
			return adj_str;
		}

		virtual void set(const std::string& str)
		{
			*this->uint_value = (uint32) atoi(str.c_str());
		}

		virtual std::string get()
		{
			std::stringstream str;
			str << *this->uint_value;
			return str.str();
		}

	private:

		uint32* uint_value;
	};
}