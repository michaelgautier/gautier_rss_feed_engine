#ifndef __gautier_rss_program__
#define __gautier_rss_program__

#include <string>

namespace gautier
{
	namespace rss_program
	{
		static const std::string program_name("Gautier RSS");
		constexpr auto program_stdout_verbosity_level = 0;

		//Starting point for the application.
		//	when this function is done, the application has completed.
		int execute();
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

