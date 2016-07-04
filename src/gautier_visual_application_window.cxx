//C++ Standard
#include <algorithm>
#include <iostream>
#include <set>
#include <queue>
#include <map>
#include <cmath>
#include <cstdlib>

//FLTK Common Base
#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/names.h>
#include <FL/fl_draw.H>

//FLTK Visuals
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Tile.H>

//Application Visuals Interface
#include "gautier_visual_application_window.hxx"
#include "gautier_diagnostics.hxx"

namespace ns_visual_application = gautier::visual_application;
namespace ns_visual_model = gautier::visual_application::visual_model;
namespace ns_diag = gautier::program::diagnostics;

using window_ptr_type = std::shared_ptr<Fl_Double_Window>;
using type_size = ns_visual_model::type_size;

//Module variables
static bool _main_event = false;

static constexpr double 
	_timed_execute_interval = 0.01
;

static constexpr type_size 
	_default_screen_list_size = 24
;

static const std::string _default_window_title = "gautier application window";

static std::string _screen_name{};

static Fl_Callback_p _default_window_callback = nullptr;

static ns_visual_model::activity _activity_past;
static ns_visual_model::callback_activity_processor _activity_processor_callback;
static ns_visual_model::list_type_ptr_screen _screen_list = nullptr;
static ns_visual_model::window_config_type _window_config;

static std::map<std::string, Fl_Widget*> _widgets_by_name;
static std::queue<ns_visual_model::activity> _activities;

static std::vector<Fl_Widget*> 
	_expired_screen_visuals,
	_screen_visuals
;

static window_ptr_type _window = nullptr;

//Module functions. Primary paths.
static void process_request(const ns_visual_model::activity& activity_past, ns_visual_model::activity& activity_now);
static void update_state(/*const ns_visual_model::activity& activity_past, */ns_visual_model::activity& activity_now);
static void timed_execute(void* data);

//Module functions. Window frame.
static void window_callback(Fl_Widget* wd);
static void configure_window(window_ptr_type window, const ns_visual_model::window_config_type& window_config);
static void set_window_size_range(window_ptr_type window, const ns_visual_model::window_config_type& window_config, ns_visual_model::window_size_type screen_size);

//Module functions. Widget level.
static void update_visuals(/*const ns_visual_model::activity& activity_past, */ns_visual_model::activity& activity_now);
static void define_visuals(ns_visual_model::unit_type_screen& screen);
static void define_visual(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se);

static void add_all_visuals_to_screen();
static void remove_all_visuals_from_screen();
static void trigger_all_visuals_to_redraw();
static void link_element_to_group(Fl_Group* active_group, Fl_Widget* candidate_member);
static void screen_visual_callback(Fl_Widget* wd);

static Fl_Widget* find_widget_by_name(const std::string& widget_name);
static ns_visual_model::unit_type_screen_element* find_screen_element_by_name(const std::string& screen_element_name);

static void link_name_to_visual(Fl_Widget* wd, const std::string& str);
static std::string get_name_of_visual(Fl_Widget* wd);
static ns_visual_model::unit_type_screen* get_screen();

static void clean_up();

template<typename visual_type> static visual_type* create_visual_into_queue(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se);
static Fl_Widget* create_visual(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se);

//Public Interface
ns_visual_model::activity 
ns_visual_application::create_activity()
{
	ns_visual_model::activity activity_value(_activity_past);

	return activity_value;
}

ns_visual_model::window_config_type 
ns_visual_application::create_config(ns_visual_model::window_size_type window_size, std::string title, ns_visual_model::visual_rgb_type background_color)
{
	_window_config = make_tuple(window_size, title, background_color);

	return _window_config;
}

ns_visual_model::list_type_ptr_screen 
ns_visual_application::create_screen_list()
{
	_screen_list.reset(new ns_visual_model::list_type_screen);
	_screen_list->reserve(_default_screen_list_size);

	_screen_visuals.reserve(_default_screen_list_size);

	return _screen_list;
}

void 
ns_visual_application::register_process_request_callback(ns_visual_model::callback_activity_processor process_request_callback)
{
	_activity_processor_callback = process_request_callback;

	return;
}

void 
ns_visual_application::activate()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	_window.reset(new Fl_Double_Window(0,0));
	_window->copy_label(_default_window_title.data());

	_default_window_callback = _window->callback();

	_window->callback(window_callback);

	configure_window(_window, _window_config);

	Fl_Group::current(nullptr);

	return;
}

int 
ns_visual_application::execute()
{
	auto return_status = 1;

	Fl::add_timeout(_timed_execute_interval, timed_execute);

	if(return_status)
	{
		_window->show();

		return_status = Fl::run();
	}
	else
	{
		std::exit(0);
	}

	return return_status;
}

const ns_visual_model::activity
ns_visual_application::get_past_activity()
{
	return _activity_past;
}

void 
ns_visual_application::reset_screen_element(const std::string& screen_element_name)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	if(!screen_element_name.empty())
	{
		//The visual element.
		auto ve = find_widget_by_name(screen_element_name);

		//The logical screen element.
		auto se = find_screen_element_by_name(screen_element_name);

		if(ve && se)
		{
			if(se->element_type == ns_visual_model::unit_type_screen_element_class::browser_view)
			{
				auto impl_ve = static_cast<Fl_Hold_Browser*>(ve);

				if(impl_ve)
				{
					impl_ve->clear();

					ve->redraw();
				}
			}
		}
	}

	return;
}

void 
ns_visual_application::set_screen_element(const std::string& screen_element_name, const std::string& value)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	if(!screen_element_name.empty())
	{
		//The visual element.
		auto ve = find_widget_by_name(screen_element_name);

		//The logical screen element.
		auto se = find_screen_element_by_name(screen_element_name);

		if(ve && se)
		{
			if(se->element_type == ns_visual_model::unit_type_screen_element_class::html_view)
			{
				auto impl_ve = static_cast<Fl_Help_View*>(ve);

				if(impl_ve)
				{
					impl_ve->clear_selection();
					impl_ve->value(nullptr);

					impl_ve->redraw();

					impl_ve->value(value.data());
				}
			}
		}
	}

	return;
}

void 
ns_visual_application::set_screen_element(const std::string& screen_element_name, const std::map<std::string, std::string>& data_items)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	auto ve = find_widget_by_name(screen_element_name);

	auto se = find_screen_element_by_name(screen_element_name);

	if(ve && se)
	{
		if(se->element_type == ns_visual_model::unit_type_screen_element_class::browser_view)
		{
			auto impl_ve = static_cast<Fl_Hold_Browser*>(ve);

			if(impl_ve)
			{
				impl_ve->clear();

				for(auto& data_item : data_items)
				{
					impl_ve->add(data_item.first.data());
				}
			}
		}
	}

	return;
}

//Module Implementation
//Private Interface
static void 
process_request(const ns_visual_model::activity& activity_past, ns_visual_model::activity& activity_now)
{
	if(activity_now.window_closing)
	{
		clean_up();
	}
	else if(_activity_processor_callback)
	{
		_activity_processor_callback(activity_past, activity_now);

		update_state(/*activity_past, */activity_now);
	}

	return;
}

static void 
update_state(/*const ns_visual_model::activity& activity_past, */ns_visual_model::activity& activity_now)
{
	update_visuals(/*activity_past, */activity_now);

	_activity_past = activity_now;

	return;
}

static void 
update_visuals(/*const ns_visual_model::activity& activity_past, */ns_visual_model::activity& activity_now)
{
	bool screen_changed = (_screen_name != activity_now.screen_name);

	//new display
	if(screen_changed)
	{
		_screen_name = activity_now.screen_name;

		std::string screen_title{};

		auto screen = get_screen();

		if(screen)
		{
			screen_title = screen->label;

			if(!screen_title.empty())
			{
				std::string window_title = (std::get<1>(_window_config) + " / " + screen_title);

				_window->copy_label(window_title.data());
			}

			//logical model is sized, the visual implementation will inherit.
			ns_visual_model::arrange_elements(screen, Fl::w(), Fl::h());

			std::vector<Fl_Widget*> screen_visuals;

			for(auto& se : screen->screen_elements)
			{
				define_visual(screen_visuals, se);
			}

			_screen_visuals = screen_visuals;

		}
	}

	return;
}

static Fl_Widget* 
find_widget_by_name(const std::string& widget_name)
{
	Fl_Widget* found = _widgets_by_name[widget_name];

	return found;
}

static ns_visual_model::unit_type_screen_element* 
find_screen_element_by_name(const std::string& screen_element_name)
{
	return ns_visual_model::find_screen_element_by_name(_screen_list, _screen_name, screen_element_name);
}

static ns_visual_model::unit_type_screen* 
get_screen()
{
	return ns_visual_model::find_screen_by_name(_screen_list, _screen_name);
}

static void 
timed_execute(void* data)
{
	bool have_updates = ((!_expired_screen_visuals.empty()) || (!_screen_visuals.empty()));

	if(!_main_event && have_updates)
	{
		     if(!_screen_visuals.empty())
		{
			add_all_visuals_to_screen();
		}
		else if(!_expired_screen_visuals.empty())
		{
			//FLTK widget removal should occur outside callbacks.
			remove_all_visuals_from_screen();
		}

		_window->redraw();
	}
	else
	{
		auto activity_now = _activity_past;

		if(!_activities.empty())
		{
			activity_now = _activities.front();
			_activities.pop();
		}

		_main_event = true;
		process_request(_activity_past, activity_now);
		_main_event = false;
	}

	Fl::repeat_timeout(_timed_execute_interval, timed_execute);

	return;
}

static void 
add_all_visuals_to_screen()
{
	for(auto screen_visual : _screen_visuals)
	{
		if(screen_visual)
		{
			if(!screen_visual->parent())
			{
				_window->add(screen_visual);
			}
		}
	}

	_screen_visuals.clear();

	return;
}

static void 
remove_all_visuals_from_screen()
{
	const auto widget_count = _window->children();

	std::vector<Fl_Widget*> screen_visuals;

	for(auto widget_index = 0; widget_index < widget_count; widget_index++)
	{
		screen_visuals.push_back(_window->child(widget_index));
	}

	for(auto screen_visual : screen_visuals)
	{
		auto fl_group = screen_visual->as_group();

		if(fl_group)
		{
			const auto fl_group_item_count = fl_group->children();

			for(auto fl_group_item_index = 0; fl_group_item_index < fl_group_item_count; fl_group_item_index++)
			{
				auto fl_group_item = fl_group->child(fl_group_item_index);

				fl_group->remove(fl_group_item);
			}
		
			_window->remove(fl_group);
		}
		else
		{
			_window->remove(screen_visual);
		}
	}

	screen_visuals.clear();

	_window->clear();

	return;
}

static void 
link_element_to_group(Fl_Group* active_group, Fl_Widget* candidate_member)
{
	if(active_group && candidate_member)
	{
		active_group->add(candidate_member);
	}

	return;
}

static void 
trigger_all_visuals_to_redraw()
{
	for(auto screen_visual : _screen_visuals)
	{
		if(screen_visual)
		{
			screen_visual->redraw();
		}
	}

	return;
}

static void 
screen_visual_callback(Fl_Widget* wd)
{
	if(wd)
	{
		ns_visual_model::activity 
		activity_now(_activity_past);

		const std::string 
		screen_element_name = get_name_of_visual(wd);

		activity_now.screen_name = _screen_name;
		activity_now.screen_element_name = screen_element_name;

		auto 
		se = find_screen_element_by_name(screen_element_name);

		if(se)
		{
			activity_now.visual_clicked = true;

			const auto 
				se_type = se->element_type
			;

			if(se_type == ns_visual_model::unit_type_screen_element_class::browser_view)
			{
				activity_now.visual_clicked = true;

				Fl_Hold_Browser* hb = 
				static_cast<Fl_Hold_Browser*>(wd);

				if(hb)
				{
					auto selected_line = hb->value();

					if(selected_line > 0)
					{
						std::string info = hb->text(selected_line);

						activity_now.data = info;
					}
				}
			}

			_activities.push(activity_now);
		}
	}

	return;
}

static void 
window_callback(Fl_Widget* wd)
{
	int event = Fl::event();

	ns_visual_model::activity activity_past = ns_visual_application::get_past_activity();

	ns_visual_model::activity 
	activity_now = ns_visual_application::create_activity();

	activity_now.window_opening = (event == FL_SHOW);
	activity_now.window_closing = (event == FL_CLOSE);
	activity_now.window_esc_key_pressed = (event == FL_SHORTCUT && Fl::event_key() == FL_Escape);

	_activities.push(activity_now);

	if(_default_window_callback && !activity_now.window_esc_key_pressed)
	{
		_default_window_callback(wd, nullptr);
	}

	return;
}

static void 
configure_window(window_ptr_type window, const ns_visual_model::window_config_type& window_config)
{
	if(_window->w() == 0 && _window->h() == 0)
	{
		auto screen_width = Fl::w();
		auto screen_height = Fl::h();

		std::string title = std::get<1>(window_config);

		window->size(screen_width, screen_height);
		window->copy_label(title.data());

		set_window_size_range(window, window_config, ns_visual_model::window_size_type(screen_width, screen_height));

		std::cout << "window max dimensions " 
		<< screen_width << " " 
		<< screen_height 
		<< "\n";
	}

	ns_visual_model::visual_rgb_type config_color = std::get<2>(window_config);

	auto r = std::get<0>(config_color);
	auto g = std::get<1>(config_color);
	auto b = std::get<2>(config_color);

	Fl_Color window_color = fl_rgb_color(r, g, b);

	window->color(window_color);

	return;
}

static void 
set_window_size_range(window_ptr_type window, const ns_visual_model::window_config_type& window_config, ns_visual_model::window_size_type screen_size)
{
	fl_font(FL_SCREEN, 10);

	auto preferred_width = std::get<0>(std::get<0>(window_config));
	auto preferred_height = std::get<1>(std::get<0>(window_config));

	auto screen_width = std::get<0>(screen_size); 
	auto screen_height = std::get<1>(screen_size);

	auto min_width = std::min(preferred_width, screen_width);
	auto min_height = std::min(preferred_height, screen_height);

	if(min_width == screen_width)
	{
		min_width = screen_width/2;
	}

	if(min_height == screen_height)
	{
		min_height = screen_height/2;
	}

	window->size_range(min_width, min_height, screen_width, screen_height);

	return;
}

static void 
link_name_to_visual(Fl_Widget* wd, const std::string& str)
{
	if(wd)
	{
		_widgets_by_name[str] = wd;
	}

	return;
}

static std::string 
get_name_of_visual(Fl_Widget* wd)
{
	std::string 
		value{}
	;

	if(wd)
	{
		for(auto data : _widgets_by_name)
		{
			if(data.second == wd)
			{
				value = data.first;

				break;
			}
		}
	}

	return value;
}

const char* link_click_callback(Fl_Widget* wd, const char* uri)
{
	if(wd)
	{
		const std::string 
			link_text = uri
		;

		std::cout << "clicked "
		<< link_text
		<< "\n";
	}

	return nullptr;//returning null prevents overwriting the link displayed.
}

static void 
define_visual(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se)
{
	Fl_Widget* ve = create_visual(screen_visuals, se);

	if(ve)
	{
		const auto 
			se_type = se.element_type
		;

		if(se_type == ns_visual_model::unit_type_screen_element_class::text_field)
		{
			auto screen_visual = static_cast<Fl_Input*>(ve);

			screen_visual->maximum_size(se.get_input_limit<int>());
		}
		else if(se_type == ns_visual_model::unit_type_screen_element_class::html_view)
		{
			auto screen_visual = static_cast<Fl_Help_View*>(ve);

			if(!se.scroll_x_on && !se.scroll_y_on)
			{
				screen_visual->scrollbar_size(1);
			}

			if(se.handles_click)
			{
				screen_visual->link(link_click_callback);
			}
		}
		else if(se_type == ns_visual_model::unit_type_screen_element_class::scroll_view)
		{
			auto screen_visual = static_cast<Fl_Scroll*>(ve);

			screen_visual->box(Fl_Boxtype::FL_THIN_DOWN_FRAME);
		}
		else if(se_type == ns_visual_model::unit_type_screen_element_class::browser_view)
		{
			auto screen_visual = static_cast<Fl_Hold_Browser*>(ve);

			if(se.handles_click)
			{
				screen_visual->callback(screen_visual_callback);
			}
		}

		if(!se.follows.empty())
		{
			auto fve = find_widget_by_name(se.follows);

			if(fve)
			{
				auto screen_group = fve->as_group();

				link_element_to_group(screen_group, ve);
			}
		}
	}

	return;
}

static 
Fl_Widget* create_visual(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se)
{
	Fl_Widget* ve = nullptr;

	const auto 
		se_type = se.element_type
	;

	if(se_type == ns_visual_model::unit_type_screen_element_class::text_field)
	{
		ve = create_visual_into_queue<Fl_Input>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::button)
	{
		ve = create_visual_into_queue<Fl_Button>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::html_view)
	{
		ve = create_visual_into_queue<Fl_Help_View>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::scroll_view)
	{
		ve = create_visual_into_queue<Fl_Scroll>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::browser_view)
	{
		ve = create_visual_into_queue<Fl_Hold_Browser>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::auto_sizer)
	{
		ve = create_visual_into_queue<Fl_Tile>(screen_visuals, se);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::vertical_stack)
	{
		ve = create_visual_into_queue<Fl_Pack>(screen_visuals, se);
		ve->type(Fl_Pack::VERTICAL);
	}
	else if(se_type == ns_visual_model::unit_type_screen_element_class::horizontal_stack)
	{
		ve = create_visual_into_queue<Fl_Pack>(screen_visuals, se);
		ve->type(Fl_Pack::HORIZONTAL);
	}

	if(ve)
	{
		link_name_to_visual(ve, se.name);
	}

	return ve;
}

template<typename visual_type>
static 
visual_type* create_visual_into_queue(std::vector<Fl_Widget*>& screen_visuals, ns_visual_model::unit_type_screen_element& se)
{
	std::string 
		screen_label{}
	;

	if(se.label_visible && !se.label.empty())
	{
		screen_label = se.label;
	}

	screen_visuals.push_back(new visual_type(se.get_x<int>(), se.get_y<int>(), se.get_w<int>(), se.get_h<int>(), screen_label.data()));

	return static_cast<visual_type*>(screen_visuals.back());
}

static void clean_up()
{
	_activities = std::queue<ns_visual_model::activity>();
	_activity_processor_callback = {};
	_activity_past = {};
	_window_config = {};
	_widgets_by_name.clear();
	_screen_list->clear();
	_screen_visuals.clear();
	_expired_screen_visuals.clear();
	_window->clear();

	return;
}

//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

