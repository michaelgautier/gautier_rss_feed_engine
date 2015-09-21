#include "gautier_rss_program.hxx"
#include "gautier_diagnostics.hxx"

int main(int argc, char * argv[]) {
	namespace ns_diag = gautier::program::diagnostics;
	namespace ns_grss = gautier::rss_program;

	if(ns_diag::trace_on)
	{
		ns_diag::write_cout_prog_head(ns_grss::program_name, ns_diag::trace_on, __FILE__, __LINE__, __func__);
		ns_diag::write_argv(argc, argv);
	}

	int result = gautier::rss_program::execute();

	if(ns_diag::trace_on)
	{
		ns_diag::write_cout({"exiting"});
	}

	return result;
}
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

