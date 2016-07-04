#ifndef __gautier_rss_model__
#define __gautier_rss_model__

#include <string>
#include <map>
#include <vector>

namespace gautier
{
	namespace rss_model
	{
		struct unit_type_rss_source
		{
			int 
				id{0},
				type_code{0}
			;

			std::string 
				name{""},
				url{""}
			;
		};

		struct unit_type_rss_item
		{
			int 
				id{0}
			;

			std::string 
				title{},
				link{},
				description{},
				pubdate{}
			;
		};

		using unit_type_list_rss_item_value = std::map<std::string, std::string>;
		using unit_type_list_rss_item = std::vector<unit_type_list_rss_item_value>;
		using unit_type_list_rss = std::map<std::string, unit_type_list_rss_item>;

		using unit_type_list_rss_source = std::map<std::string, unit_type_rss_source>;

		//Should always call this at least once before any other function in this module.
		void 
		load_feeds_source_list(const std::string& feeds_list_file_name, unit_type_list_rss_source& feed_sources);

		//Reloads source list from cache with updated expiration indicator.
		void 
		load_feeds_source_list(unit_type_list_rss_source& feed_sources);

		//Collects and saves feeds.
		//Gathered feed items can be retrieved more selectively by the application.
		//*Recommended way to gather feed items.
		void 
		collect_feeds(const unit_type_list_rss_source& feed_sources);

		//Collects and saves feeds and returns the list of all feed items collected.
		//Best used for caching all feed items for all feed sources.
		//Simplest way to execute the rss feed engine but accumulates all data into memory.
		//Most useful for running full tests of the overall feed gather and output process.
		void 
		collect_feeds(const unit_type_list_rss_source& feed_sources, unit_type_list_rss& rss_feed_items);

		//Returns all rss feed items previously collected.
		//Useful for caching all feeds items previously collected.
		void 
		load_feeds(unit_type_list_rss& rss_feed_items);

		//Returns all rss feed items previously collected for an rss feed source.
		//*Recommended way to access feed items after collecting them.
		void 
		load_feed(const unit_type_rss_source& feed_source, unit_type_list_rss& rss_feed_items);

		//Returns all rss feed items previously collected for an rss feed source.
		//Provides a convenient way to access feed items after collecting them.
		//Matches feeds by name of the feed source.
		void 
		load_feed(const std::string feed_source_name, unit_type_list_rss& rss_feed_items);

		void 
		create_feed_items_list(const unit_type_list_rss& rss_feed_items, std::vector<unit_type_rss_item>& rss_items);

		//output to std out.
		//terminal output.
		//possible, future output to html file.
		void 
		output_feeds(const unit_type_list_rss& rss_feed_items);
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

