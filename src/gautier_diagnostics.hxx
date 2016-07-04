#ifndef __gautier_program_diagnostics__
#define __gautier_program_diagnostics__

#include <string>
#include <vector>
#include <initializer_list>

namespace gautier
{
	namespace program
	{
		namespace diagnostics
		{
			constexpr auto trace_on = true;
			constexpr auto verbosity_level = 0;
			//5 = technical detail, 4, little tech detail, 0 - 3 std output
			//define enumerations later on.

			void write_cout(const std::vector<std::string>& values);
			void write_cout(const std::initializer_list<const std::string>& values);
			void write_cout_prog_head(const std::string programname, const bool& cpp_std_shown, const std::string& filename, const int& linenumber, const std::string& functionname);
			void write_cout_prog_unit(const std::string& filename, const int& linenumber, const std::string& functionname);
			void write_argv(int argc, char * argv[]);
		}
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

