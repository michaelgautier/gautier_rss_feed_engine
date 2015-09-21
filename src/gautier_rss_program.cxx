#include <iostream>

#include "gautier_rss_program.hxx"
#include "gautier_rss_model.hxx"

//Michael Gautier
//Initial model, gautier_rss_model.cxx, completed: 9/20/2015 11:11 PM.

//This module is a developmental scaffold. 9/9/2015
//This module is of a temporary nature to test and refine the gautier_rss_model engine
//	The engine can be incorporated in any process with the initial intended purpose
//	to bind it with a user interface.
//This module presents output to the command-line. The Gnome Terminal provides the ability 
//	to activate links directly from the output.
//The pattern implied in the output_feeds function can be adapted to create various 
//	text based outputs such as HTML for example. That output type was not part 
//	of the initial design since presentation in a 
//	self-contained user interface program was the 'only' priority.
namespace ns_grss = gautier::rss_program;

int ns_grss::execute()
{
	auto return_status = 0;

	namespace ns_grss_model = gautier::rss_model;

	//MEMORY:
	//A test data set involving 19 files and 7,506 strings on 9/14/2015.
	//Total RAM used, 21 Megabytes.
	//Valgrind tests show no memory leaks at the application level.

	//Optimizations considered:
	//Memory use by swapping disk for memory. Not a substantial gain at this time.
	//Program speed up using parallel retrieval. Three seconds is okay.

	//The following is the basic template for rss retrieval. 
	//This is what the top level of the program should 
	//look like. This is also how a refresh call would work.
	ns_grss_model::
	unit_type_list_rss_source rss_feed_sources;

	ns_grss_model::
	unit_type_list_rss rss_feeds;

	//start with an empty list.
	if(rss_feed_sources.empty())
	{
		const std::string //Could be parameterized from command line.
		rss_feeds_sources_file_name = "gautier_rss_feeds_test.txt";

		ns_grss_model::
		load_feeds_source_list(rss_feeds_sources_file_name, rss_feed_sources);
	}

	//once the list has entries, let's process it.
	if(!rss_feed_sources.empty() && rss_feeds.empty())
	{
		//processes the feeds, saves them, sets feed items in rss_feeds.
		ns_grss_model::
		collect_feeds(rss_feed_sources, rss_feeds);
	}
	else
	{
		std::cout << "no rss_feed_sources\n";
	}

	//TIME: At this point, pulling from a list of files/embedded database, 
	//	runs in 3/4 of a second.
	
	//Not timed for network communications. Will not be timed since that is dependent on the ISP.

	//When we have feeds data, let's show it.
	if(!rss_feeds.empty())
	{
		ns_grss_model::
		output_feeds(rss_feeds);
	}
	else
	{
		std::cout << "no rss_feeds data\n";
	}
	//TIME: After this point, if say 10,000 entries were output to std-out, about 3 to 4 seconds.
	//As API for the user interface, which will not be outputting to console, this is not important.
	//Only an important detail if there was a case for outputting all data to a file.

	return return_status;
}
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

