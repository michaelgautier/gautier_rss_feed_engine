#include <iostream>
#include <memory>

#include "gautier_diagnostics.hxx"

namespace ns_diag = gautier::program::diagnostics;

void 
ns_diag::write_cout(const std::vector<std::string>& values)
{
	if(trace_on && verbosity_level >= 0)
	{
		for(const std::string& value : values)
		{
			std::cout << value;
		}
	}

	return;
}

void 
ns_diag::write_cout(const std::initializer_list<const std::string>& values)
{
	if(trace_on && verbosity_level >= 0)
	{
		for(const std::string& value : values)
		{
			std::cout << value;
		}
	}

	return;
}

void 
ns_diag::write_cout_prog_unit(const std::string& filename, const int& linenumber, const std::string& functionname)
{
	if(trace_on && verbosity_level >= 4)
	{
		const std::initializer_list<const std::string> values({filename, "(", std::to_string(linenumber), ") ", functionname, "\n"});

		write_cout(values);
	}

	return;
}

void 
ns_diag::write_cout_prog_head(const std::string programname, const bool& cpp_std_shown, const std::string& filename, const int& linenumber, const std::string& functionname)
{
	if(trace_on && verbosity_level >= 5)
	{
		write_cout_prog_unit(filename, linenumber, functionname);

		std::cout << programname << "\n";

		if(cpp_std_shown)
		{
			std::cout << "  C++ STD rev. " << __cplusplus << "\n";
			std::cout << "  C++ LIB hosted " << (__STDC_HOSTED__ ? "yes" : "no") << "\n";
			std::cout << "  program build date " << __DATE__ << " " << __TIME__ << "\n";
		}
	}

	return;
}

void 
ns_diag::write_argv(int argc, char * argv[])
{
	if(trace_on && verbosity_level >= 5 && argc > 0)
	{
		using n_unit = unsigned;

		n_unit arg_len = static_cast<n_unit>(argc);

		std::vector<std::string> diag_output;
		diag_output.reserve(arg_len);

		for(n_unit cnt = 0; cnt < arg_len; cnt++)
		{
			const std::string value(argv[cnt]);

			diag_output.push_back(value);
		}

		write_cout(diag_output);
	}

	return;
}

//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

