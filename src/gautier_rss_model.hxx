#ifndef __gautier_rss_model__
#define __gautier_rss_model__

#include <string>
#include <map>
#include <vector>

//Michael Gautier
//Initial model completed: 9/20/2015 11:11 PM.
namespace gautier
{
	namespace rss_model
	{
		using unit_type_list_rss_item_value = std::map<std::string, std::string>;
		using unit_type_list_rss_item = std::vector<unit_type_list_rss_item_value>;
		using unit_type_list_rss = std::map<std::string, unit_type_list_rss_item>;

		//this could be better specified, but will do for now.
		using unit_type_list_rss_source = std::map<std::string, std::string>;

		void 
		load_feeds_source_list(const std::string& feeds_list_file_name, unit_type_list_rss_source& feed_sources);

		void 
		collect_feeds(const unit_type_list_rss_source& feed_sources, unit_type_list_rss& rss_feeds);

		//output to std out.
		//terminal output.
		//possible, future output to html file.
		void 
		output_feeds(const unit_type_list_rss& rss_feeds);
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

