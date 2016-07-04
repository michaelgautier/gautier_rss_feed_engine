#include <algorithm>
#include <iostream>
#include <deque>
#include <queue>
#include <memory>
#include <sstream>
#include <functional>

#include <json/json.h>

#include "gautier_visual_model.hxx"
#include "gautier_screen_structure_build_json.hxx"
#include "gautier_diagnostics.hxx"

namespace ns_visual_model = gautier::visual_application::visual_model;
namespace ns_visual_build_json = gautier::visual_application::visual_model::screen_build_json;
namespace ns_diag = gautier::program::diagnostics;

//These functions requires a reader for the JSON data format.
//Other functions can be added that support other formats.
//A top level function can be defined such that it takes a data format parameter.
//	It would wrap these functions such that any data format could be supported.
//		Such a function takes a unit_type_screen, input format flag and updates the unit_type_screen.
//	Since a perceived need for this does not exist in this program, it is set aside.

/*Reads result from JsonCpp converts it to list structure used in application*/

static void link_screen_element_to_screen(ns_visual_model::unit_type_screen& screen, const int lvl, const Json::Value& json_structure);

//Applies the result from JsonCpp to a list structure used in the application.*/
static void link_screen_element_to_screen(ns_visual_model::unit_type_screen& screen, const std::string name, const std::string value);

//MAIN function for structure build and translation.
//PUBLIC ENTRY POINT.
ns_visual_model::unit_type_screen 
ns_visual_build_json::create_screen(const std::function<const std::string()> layout_data_getter)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	//PARSE
	auto json_parsed = false;

	Json::Value screen_layout_data;
	{
		Json::CharReaderBuilder reader_builder;
		reader_builder["collectComments"] = false;
		reader_builder["strictRoot"] = true;
		reader_builder["failIfExtra"] = true;
		reader_builder["rejectDupKeys"] = true;

		std::string reader_error_data;

		std::istringstream stream_input(layout_data_getter());

		json_parsed = Json::parseFromStream(reader_builder, stream_input, &screen_layout_data, &reader_error_data);

		if(!json_parsed)
		{
			std::cout << __FILE__ << " (" << __LINE__ << ") json data not parsed: "
			<< reader_error_data << "\n";
		}
	}

	//MAP INTERNAL
	unit_type_screen screen;
	screen.screen_elements.reserve(32);

	if(json_parsed)
	{
		constexpr int initial_screen_layout_level = 0;

		link_screen_element_to_screen(screen, initial_screen_layout_level, screen_layout_data);
	}

	return screen;
}

static void 
link_screen_element_to_screen(ns_visual_model::unit_type_screen& screen, const int lvl, const Json::Value& json_structure)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	for(Json::Value::const_iterator json_field = json_structure.begin(); json_field != json_structure.end(); std::advance(json_field, 1))
	{
		const auto node_key = json_field.key();
		const uint node_type = json_field->type();

		const bool is_container = (
		   node_type == Json::ValueType::objectValue 
		|| node_type == Json::ValueType::arrayValue
		);

		if(is_container)
		{
			if(lvl == 2)
			{
				screen.screen_elements.push_back(ns_visual_model::unit_type_screen_element());
			}

			link_screen_element_to_screen(screen, (lvl + 1), *json_field);
		}
		else
		{
			const std::string key_name = node_key.asString();
			const std::string value = json_field->asString();

			link_screen_element_to_screen(screen, key_name, value);
		}
	}

	return;
}

static void 
link_screen_element_to_screen(ns_visual_model::unit_type_screen& screen, const std::string name, const std::string value)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	if(name == "screen_name")
	{
		screen.name = value;
	}
	else if(name == "screen_label")
	{
		screen.label = value;
	}
	else if(name == "screen_element_name")
	{
		screen.screen_elements.back().name = value;
	}
	else if(name == "screen_element_label")
	{
		screen.screen_elements.back().label = value;
	}
	else if(name == "follows")
	{
		screen.screen_elements.back().follows = value;
	}
	else if(name == "screen_element_label_visible")
	{
		screen.screen_elements.back().label_visible = (value == "true");
	}
	else if(name == "receives_input")
	{
		screen.screen_elements.back().receives_input = (value == "true");
	}
	else if(name == "scroll_x_on")
	{
		screen.screen_elements.back().scroll_x_on = (value == "true");
	}
	else if(name == "scroll_y_on")
	{
		screen.screen_elements.back().scroll_y_on = (value == "true");
	}
	else if(name == "handles_click")
	{
		screen.screen_elements.back().handles_click = (value == "true");
	}
	else if(name == "input_limit")
	{
		screen.screen_elements.back().input_limit = std::stoi(value);
	}
	else if(name == "x")
	{
		screen.screen_elements.back().x = std::stoi(value);
	}
	else if(name == "y")
	{
		screen.screen_elements.back().y = std::stoi(value);
	}
	else if(name == "w")
	{
		screen.screen_elements.back().w = std::stoi(value);
	}
	else if(name == "h")
	{
		screen.screen_elements.back().h = std::stoi(value);
	}
	else if(name == "screen_element_type")
	{
		screen.screen_elements.back().element_type = ns_visual_model::unit_type_screen_element_class_text::create_type(value);
	}
	else
	{
		std::cout << __FILE__ << " (" << __LINE__ << ") unrecognized element: " 
		<< name << " = " << value << "\n";
	}

	return;
}

