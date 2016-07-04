#include <iostream>
#include <fstream>

#include "gautier_diagnostics.hxx"
#include "gautier_rss_model.hxx"
#include "gautier_rss_program.hxx"
#include "gautier_screen_structure_build.hxx"
#include "gautier_visual_application_window.hxx"
#include "gautier_visual_model.hxx"

namespace ns_visual_application = gautier::visual_application;
namespace ns_visual_model = gautier::visual_application::visual_model;
namespace ns_grss = gautier::rss_program;
namespace ns_diag = gautier::program::diagnostics;
namespace ns_grss_model = gautier::rss_model;

//Module variables
static std::vector<ns_grss_model::unit_type_rss_item> 
	_rss_items
;

static constexpr int 
	_default_screen_list_size = 24,
	_default_window_width = 1024,
	_default_window_height = 768
;

static auto 
	_default_window_background_color = {0, 170, 172}//some kind of blue.
;

//Top-level Implementation Interface
static void process_request(const ns_visual_model::activity& activity_past, ns_visual_model::activity& activity_now);
static void process_request_detail(const ns_visual_model::activity& activity_past);

static void fill_screen_list(ns_visual_model::list_type_ptr_screen);

static const std::string get_screen_layout_data_main_screen();
static const std::string get_screen_layout_data_empty_screen();

//Application Entry Point
//This is the general skeleton of the program.
//	Designed to work in all desktop operating environments.
int ns_grss::execute()
{
	bool test = false;
	if(!test)
	{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	ns_visual_model::window_size_type 
	window_default_size(_default_window_width, _default_window_height);

	auto 
	window_background_color = ns_visual_model::create_rgb_color(_default_window_background_color);

	ns_visual_application::register_process_request_callback(process_request);

	ns_visual_application::create_config(window_default_size, program_name, window_background_color);

	fill_screen_list(ns_visual_application::create_screen_list());

	ns_visual_application::activate();

	auto return_status = ns_visual_application::execute();

	return return_status;
	}
	else
	{

	ns_visual_model::list_type_ptr_screen screen_list;
	screen_list.reset(new ns_visual_model::list_type_screen);

	fill_screen_list(screen_list);

	ns_visual_model::arrange_elements(&screen_list->front(), 1366, 707);

	return 0;

	}
}

//General, Top-level Implementation
static void 
process_request(const ns_visual_model::activity& activity_past, ns_visual_model::activity& activity_now)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	//process pending events first.
	//... the past cannot be changed.
	if(!activity_past.forwarded_event_tag.empty())
	{
		process_request_detail(activity_past);

		activity_now.forwarded_event_tag = "";
	}

	//run next line of logic next.
	//... only the present.
	if(activity_now.visual_clicked)
	{
		activity_now.visual_clicked = false;

		const std::string screen_name = activity_now.screen_name;
		const std::string screen_element_name = activity_now.screen_element_name;

		if(screen_name == "feeds_screen")
		{
			const std::string info_item = activity_now.data;

			if(screen_element_name == "rss_feed_sources_view")
			{
				if(!info_item.empty())
				{
					activity_now.forwarded_event_tag = "feeds_screen load individual feed";
				}
			}
			else if(screen_element_name == "rss_feed_lines_view")
			{
				if(!info_item.empty())
				{
					activity_now.forwarded_event_tag = "feeds_screen load individual feed description";
				}
			}
		}
	}

	//default logic to initiate the process.
	if(activity_past.screen_name.empty())
	{
		activity_now.screen_name = "feeds_screen";
		activity_now.forwarded_event_tag = "feeds_screen load data";
	}

	return;
}

static void 
process_request_detail(const ns_visual_model::activity& activity_past)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	//Specific scenarios keyed on a string description.
	if(activity_past.forwarded_event_tag == "feeds_screen load data")
	{
		std::map<std::string, std::string> 
		data_items;

		static const 
		std::string 
		rss_feeds_sources_file_name = "gautier_rss_feeds_test.txt";

		ns_grss_model::unit_type_list_rss_source 
		rss_feed_sources;

		ns_grss_model::load_feeds_source_list(rss_feeds_sources_file_name, rss_feed_sources);

		for(const auto& rss_feed_source : rss_feed_sources)
		{
			data_items[rss_feed_source.second.name] = rss_feed_source.second.url;
		}

		if(!data_items.empty())
		{
			ns_visual_application::set_screen_element("rss_feed_sources_view", data_items);

			ns_grss_model::collect_feeds(rss_feed_sources);
		}
	}
	else if(activity_past.forwarded_event_tag == "feeds_screen load individual feed")
	{
		std::map<std::string, std::string> 
		data_items;

		const std::string 
		info_item = activity_past.data;

		if(!info_item.empty())
		{
			{
				ns_grss_model::unit_type_list_rss 
				rss_feed_items;

				ns_grss_model::load_feed(info_item, rss_feed_items);

				_rss_items.clear();

				ns_grss_model::create_feed_items_list(rss_feed_items, _rss_items);
			}

			for(const auto& rss_item : _rss_items)
			{
				data_items[rss_item.title] = rss_item.link;
			}
		}

		if(!data_items.empty())
		{
			ns_visual_application::set_screen_element("rss_feed_lines_view", data_items);
		}
	}
	else if(activity_past.forwarded_event_tag == "feeds_screen load individual feed description")
	{
		const std::string 
		item_title = activity_past.data;

		ns_grss_model::unit_type_rss_item
		found_rss_item;

		for(auto& rss_item : _rss_items)
		{
			if(rss_item.title == item_title)
			{
				found_rss_item = rss_item;

				break;
			}
		}

		if(!found_rss_item.description.empty())
		{
			const std::string 
				link = "<a href='" + found_rss_item.link + "'>view article</a>";
			;

			ns_visual_application::set_screen_element("rss_feed_link_view", link);
			ns_visual_application::set_screen_element("rss_feed_description_view", found_rss_item.description);
		}
	}

	return;
}

//Default screen definitions in JSON format.
static const 
std::string get_screen_layout_data_main_screen()
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	ns_visual_model::unit_type_screen_element_class_text 
	element_type;

	return 
	"[		\
		{	\
			\"screen_name\": \"feeds_screen\", \
			\"screen_label\": \"Feeds\", \
			 \"elements\": [\
				{ \
					\"screen_element_name\": \"main_canvas\", \
					\"screen_element_label_visible\": false, \
					\"screen_element_type\": " + element_type.auto_sizer + " \
				},\
				{ \
					\"screen_element_name\": \"rss_feed_names_lane\", \
					\"screen_element_label_visible\": false, \
					\"follows\": \"main_canvas\", \
					\"w\": 280.00, \
					\"screen_element_type\": " + element_type.vertical_stack + " \
				},\
				{ \
					\"screen_element_name\": \"rss_details_lane\", \
					\"screen_element_label_visible\": false, \
					\"follows\": \"main_canvas\", \
					\"screen_element_type\": " + element_type.vertical_stack + " \
				},\
				{ \
					\"screen_element_name\": \"rss_feed_sources_view\", \
					\"screen_element_label\": \"rss feeds\", \
					\"screen_element_label_visible\": true, \
					\"follows\": \"rss_feed_names_lane\", \
					\"screen_element_type\": " + element_type.browser_view + " \
				},\
				{ \
					\"screen_element_name\": \"rss_feed_lines_view\", \
					\"screen_element_label\": \"latest\", \
					\"screen_element_label_visible\": true, \
					\"follows\": \"rss_details_lane\", \
					\"h\": 464.00, \
					\"screen_element_type\": " + element_type.browser_view + " \
				},\
				{ \
					\"screen_element_name\": \"rss_feed_link_view\", \
					\"screen_element_label\": \"link\", \
					\"screen_element_label_visible\": false, \
					\"scroll_x_on\": false, \
					\"scroll_y_on\": false, \
					\"follows\": \"rss_details_lane\", \
					\"h\": 24.00, \
					\"screen_element_type\": " + element_type.html_view + " \
				},\
				{ \
					\"screen_element_name\": \"rss_feed_description_view\", \
					\"screen_element_label\": \"details\", \
					\"screen_element_label_visible\": true, \
					\"handles_click\": false, \
					\"follows\": \"rss_details_lane\", \
					\"screen_element_type\": " + element_type.html_view + " \
				}\
			]\
		}	\
	]		\
	";
}

static const 
std::string get_screen_layout_data_empty_screen()
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	ns_visual_model::unit_type_screen_element_class_text 
	element_type;

	return 
	"[		\
		{	\
			\"screen_name\": \"emptyscreen\", \
			\"screen_label\": \"Break Screen \", \
			 \"elements\": [\
				{ \
					\"screen_element_name\": \"empty_item\", \
					\"screen_element_label\": \" \", \
					\"screen_element_label_visible\": false, \
					\"receives_input\": false, \
					\"input_limit\": 0, \
					\"x\": 10.00, \
					\"y\": 08.00, \
					\"w\": 450.00, \
					\"h\": 24.00, \
					\"screen_element_type\": " + element_type.text_field + " \
				}\
			]\
		}	\
	]		\
	";
}

static void 
fill_screen_list(ns_visual_model::list_type_ptr_screen screen_list)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	ns_visual_model::screen_build::list_type_screen_layout_data_getter 
	screen_descriptions = {
		get_screen_layout_data_main_screen,
		get_screen_layout_data_empty_screen
	};

	ns_visual_model::screen_build::get_all_screens(screen_list, screen_descriptions);

	for(auto& screen : *screen_list)
	{
		screen.screen_elements.reserve(_default_screen_list_size);
	}

	return;
}

//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

