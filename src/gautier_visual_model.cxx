#include <algorithm>
#include <iostream>
#include <map>

#include "gautier_visual_model.hxx"
#include "gautier_diagnostics.hxx"

namespace ns_visual_model = gautier::visual_application::visual_model;
namespace ns_diag = gautier::program::diagnostics;

using list_type_follows_count = std::map<std::string, ns_visual_model::type_size_ar>;

static void count_follows(const ns_visual_model::list_type_screen_element& screen_elements, list_type_follows_count& follows_count);

ns_visual_model::visual_rgb_type 
ns_visual_model::create_rgb_color(std::initializer_list<int>& color_components)
{
	int 
		r{},
		g{},
		b{}
	;

	int c_idx = -1;

	for(auto color_component : color_components)
	{
		c_idx++;

		if(c_idx == 0)
		{
			r = color_component;
		}
		else if(c_idx == 1)
		{
			g = color_component;
		}
		else if(c_idx == 2)
		{
			b = color_component;
		}
	}

	return visual_rgb_type(r, g, b);
}

void 
ns_visual_model::copy_screen_elements(const unit_type_screen& src_screen, list_type_ptr_screen dest_screens)
{
		const std::string screen_name = src_screen.name;

		auto dest_screen = 
		std::find_if(
				  dest_screens->begin()
				, dest_screens->end()

				,[&screen_name] (const unit_type_screen& value)
				{
					return (value.name == screen_name);
				}
		);

		if(dest_screen != dest_screens->end())
		{
			std::copy(src_screen.screen_elements.cbegin()
				, src_screen.screen_elements.cend()
				, dest_screen->screen_elements.begin()
			);
		}
		else
		{
			dest_screens->push_back(src_screen);
		}

	return;
}

void 
ns_visual_model::insert_screen_elements(const unit_type_screen_element_in_screen_name_pair& parent_child_names, list_type_ptr_screen screens, list_type_screen_element new_elements)
{
	auto source_screen = 
	std::find_if(
			  screens->begin()
			, screens->end()

			,[&parent_child_names] (const ns_visual_model::unit_type_screen& value)
			{
				return (value.name == parent_child_names.screen_name);
			}
	);

	if(source_screen != screens->end())
	{
		auto source_screen_element =
		std::find_if(
				  source_screen->screen_elements.begin()
				, source_screen->screen_elements.end()
				
				, [&parent_child_names] (const ns_visual_model::unit_type_screen_element& value)
				{
					return (value.name == parent_child_names.screen_element_name);
				}
		);

		if(source_screen_element != source_screen->screen_elements.cend())
		{
			auto size_prior = source_screen->screen_elements.size();
			source_screen->screen_elements.insert(source_screen_element, new_elements.cbegin(), new_elements.cend());
			source_screen->changed = source_screen->screen_elements.size() != size_prior;
		}
	}

	return;
}

bool 
ns_visual_model::is_screen_changed (const ns_visual_model::unit_type_screen value) 
{
	return value.changed;
}

long 
ns_visual_model::count_changed_screens (ns_visual_model::list_type_ptr_screen screens)
{
	return 
	std::count_if(screens->cbegin(), screens->cend(), is_screen_changed);
}

ns_visual_model::unit_type_screen* 
ns_visual_model::find_changed_screens (ns_visual_model::list_type_ptr_screen screens)
{
	ns_visual_model::unit_type_screen* screen = nullptr;

	auto found = std::find_if(screens->begin(), screens->end(), is_screen_changed);
	
	if(found != screens->end())
	{
		screen = &(*found);
	}

	return screen;
}

ns_visual_model::unit_type_screen* 
ns_visual_model::find_screen_by_name (ns_visual_model::list_type_ptr_screen screens, const std::string& name)
{
	ns_visual_model::unit_type_screen* 
	screen = nullptr;

	auto screen_finder = [&name](const ns_visual_model::unit_type_screen& value)
	{
		return value.name == name;
	};

	auto found = std::find_if(screens->begin(), screens->end(), screen_finder);
	
	if(found != screens->end())
	{
		screen = &(*found);
	}

	return screen;
}

ns_visual_model::unit_type_screen_element* 
ns_visual_model::find_screen_element_by_name(ns_visual_model::list_type_ptr_screen screens, const std::string& screen_name, const std::string& screen_element_name)
{
	ns_visual_model::unit_type_screen_element* 
	screen_element = nullptr;

	auto source_screen = 
	find_screen_by_name(screens, screen_name);

	if(source_screen)
	{
		screen_element =
		find_screen_element_by_name(source_screen->screen_elements, screen_element_name);
	}

	return screen_element;
}

ns_visual_model::unit_type_screen_element* 
ns_visual_model::find_screen_element_by_name(ns_visual_model::list_type_screen_element& screen_elements, const std::string& screen_element_name)
{
	ns_visual_model::unit_type_screen_element* 
	screen_element = nullptr;

	auto source_screen_element =
	std::find_if(
			  screen_elements.begin()
			, screen_elements.end()
			
			, [&screen_element_name] (const ns_visual_model::unit_type_screen_element& value)
			{
				return (value.name == screen_element_name);
			}
	);

	if(source_screen_element != screen_elements.cend())
	{
		screen_element = &(*source_screen_element);
	}

	return screen_element;
}

void 
ns_visual_model::
axis_modify_x_right_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = (src.w - rel.w) + src.x;

	rel.x = value;

	return;
}

void 
ns_visual_model::
axis_modify_x_center_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = ((src.w / 2) + src.x) - (rel.w / 2);

	rel.x = value;

	return;
}

void 
ns_visual_model::
axis_modify_y_space_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = (src.y - src.h) - rel.magnitude;

	rel.y = value;

	return;
}

void 
ns_visual_model::
axis_modify_y_center_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = (src.y) - ((src.h / 2) - (rel.h / 2));

	rel.y = value;

	return;
}

void 
ns_visual_model::
axis_modify_y_top_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = (src.y + src.h) - rel.h;

	rel.y = value;

	return;
}

void 
ns_visual_model::
axis_modify_y_bottom_align(const ns_visual_model::unit_type_area& src, ns_visual_model::unit_type_area& rel)
{
	const auto value = (src.y + src.h);

	rel.y = value;

	return;
}

void 
ns_visual_model::arrange_elements(ns_visual_model::unit_type_screen* screen, const type_size_ar screen_width, const type_size_ar screen_height)
{
	const type_size_ar min_v = 0;

	list_type_follows_count 
		follows_count
	;

	//const auto se_type_auto_sizer = unit_type_screen_element_class::auto_sizer;
	//const auto se_type_horizontal_stack = unit_type_screen_element_class::horizontal_stack;
	const auto se_type_vertical_stack = unit_type_screen_element_class::vertical_stack;

	if(screen)
	{
		count_follows(screen->screen_elements, follows_count);

		unit_type_screen_element 
			previous_se
		;

		unit_type_screen_element 
			parent_se
		;

		auto 
			parent_se_type = parent_se.element_type
		;

		auto 
			rem_a = parent_se.get_area();
		;

		auto 
			tmp_count = 0
		;

		for(auto& se : screen->screen_elements)
		{
			const std::string 
				follows = se.follows
			;

			const auto 
				count = follows_count[follows]
			;

			auto area = se.get_area();

			if(follows.empty() || follows == screen->name)
			{

				if(area.w <= min_v)
				{
					area.w = screen_width;
				}

				if(area.h <= min_v)
				{
					area.h = screen_height;
				}
			}
			else
			{
				if(parent_se.name != follows)
				{
					auto found_se = find_screen_element_by_name(screen->screen_elements, follows);

					if(found_se)
					{
						parent_se = *found_se;
						parent_se_type = parent_se.element_type;

						rem_a = parent_se.get_area();
						previous_se = {};
						tmp_count = count;
					}
					else
					{
						continue;
					}
				}

				bool previous_se_follows_parent = (previous_se.follows == follows);
				const auto previous_area = previous_se.get_area();
//Explict sizes defined are used as is.
				if(area.x <= min_v)
				{
					if((parent_se_type == se_type_vertical_stack))
					{
						area.x = previous_area.x;
					}
					else if(previous_se_follows_parent)
					{
						axis_modify_x_right_align(previous_area, area);
					}
					else
					{
						area.x = min_v;
					}
				}

				if(area.y <= min_v)
				{
					if((parent_se_type == se_type_vertical_stack))
					{
						axis_modify_y_bottom_align(previous_area, area);
					}
					else if(previous_se_follows_parent)
					{
						area.y = previous_area.y;
					}
					else
					{
						area.y = min_v;
					}
				}

				if(area.w <= min_v)
				{
					if((parent_se_type == se_type_vertical_stack))
					{
						area.w = parent_se.w;
					}
					else
					{
						area.w = (rem_a.w/tmp_count);
					}
				}

				rem_a.w -= area.w;

				if(area.h <= min_v)
				{
					if((parent_se_type == se_type_vertical_stack))
					{
						area.h = (rem_a.h/tmp_count);
					}
					else
					{
						area.h = parent_se.h;
					}
				}

				rem_a.h -= area.h;

			}

			se.set_area(area);

			previous_se = se;

			tmp_count--;
		}
	}

	return;
}

static void 
count_follows(const ns_visual_model::list_type_screen_element& screen_elements, list_type_follows_count& follows_count)
{
	for(const auto& screen_element : screen_elements)
	{
		const std::string 
			follows = screen_element.follows
		;

		auto count = follows_count[follows];

		count++;

		follows_count[follows] = count;
	}

	return;
}

//Debug output for the screen layout text.
//	Not used presently. Useful for quick, console based troubleshooting.
void 
ns_visual_model::print_defined_screens(list_type_ptr_screen screen_list)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	for(auto screen : *screen_list)
	{
		std::cout << "(" << __LINE__ << ") "
		 << "querying screen " << screen.name << " " << screen.label << " \n";

		for(auto screen_element : screen.screen_elements)
		{
			std::cout << "(" << __LINE__ << ") "
			<< "screen_element " << screen_element.name << " " 
			<< screen_element.label 
			<< " x/y/w/h input(y/n) limit " << screen_element.x << " " 
			<< screen_element.y << " " 
			<< screen_element.w << " " 
			<< screen_element.h << " " 
			<< screen_element.receives_input << " " 
			<< screen_element.input_limit << "\n";
		}
	}

	return;
}

//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

