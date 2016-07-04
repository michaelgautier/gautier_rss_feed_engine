#ifndef __gautier_visual_application_visual_model__
#define __gautier_visual_application_visual_model__

#include <functional>
#include <memory>
#include <tuple>
#include <vector>
#include <iostream>
#include <initializer_list>

namespace gautier
{
	namespace visual_application
	{
		namespace visual_model
		{
			using type_size = std::vector<char>::size_type;
			using type_size_ar = int;

			using window_size_type = std::tuple<int, int>;
			using visual_rgb_type = std::tuple<unsigned char, unsigned char, unsigned char>;
			using window_config_type = std::tuple<window_size_type, std::string, visual_rgb_type>;

			enum unit_type_screen_element_class : type_size
			{
				none = 0,
				text_field = 14,
				button,
				html_view,
				scroll_view,
				browser_view,
				auto_sizer,
				horizontal_stack,
				vertical_stack
			};

			struct unit_type_screen_element_class_text
			{
				std::string 
					button = std::to_string(static_cast<type_size>(unit_type_screen_element_class::button)),
					html_view = std::to_string(static_cast<type_size>(unit_type_screen_element_class::html_view)),
					scroll_view = std::to_string(static_cast<type_size>(unit_type_screen_element_class::scroll_view)),
					text_field = std::to_string(static_cast<type_size>(unit_type_screen_element_class::text_field)),
					auto_sizer = std::to_string(static_cast<type_size>(unit_type_screen_element_class::auto_sizer)),
					vertical_stack = std::to_string(static_cast<type_size>(unit_type_screen_element_class::vertical_stack)),
					horizontal_stack = std::to_string(static_cast<type_size>(unit_type_screen_element_class::horizontal_stack)),
					browser_view = std::to_string(static_cast<type_size>(unit_type_screen_element_class::browser_view))
				;

				static 
				unit_type_screen_element_class 
				create_type(const std::string& text_value)
				{
					auto nvalue = std::stoi(text_value);
					auto tvalue = static_cast<unit_type_screen_element_class>(nvalue);

					return tvalue;
				}
			};

			struct unit_type_area
			{
				unit_type_area(const type_size_ar& in_x, const type_size_ar& in_y, const type_size_ar& in_w, const type_size_ar& in_h, const type_size_ar& in_m)
				{
					x = in_x;
					y = in_y;
					w = in_w;
					h = in_h;
					magnitude = in_m;

					return;
				}

				unit_type_area() = default;
				unit_type_area(const unit_type_area& in) = default;
				unit_type_area& operator=(const unit_type_area&) = default;

				type_size_ar 
					x{0},
					y{0},
					w{0},
					h{0},
					magnitude{0}
				;
			};

			struct unit_type_screen_element
			{
				unit_type_screen_element(const unit_type_screen_element& in)
				{
					receives_input = in.receives_input;
					label_visible = in.label_visible;
					scroll_x_on = in.scroll_x_on;
					scroll_y_on = in.scroll_y_on;
					handles_click = in.handles_click;

					element_type = in.element_type;

					x = in.x;
					y = in.y;
					w = in.w;
					h = in.h;

					input_limit = in.input_limit;

					state_code = in.state_code;
					scroll_x = in.scroll_x;
					scroll_y = in.scroll_y;
					line = in.line;

					name = in.name;
					label = in.label;
					text_value = in.text_value;
					follows = in.follows;

					return;
				}

				bool 
					receives_input{false},
					label_visible{true},
					scroll_x_on{true},
					scroll_y_on{true},
					handles_click{true}
				;

				unit_type_screen_element_class 
					element_type{unit_type_screen_element_class::text_field}
				;

				type_size_ar 
					x{0},
					y{0},
					w{0},
					h{0},
					input_limit{16},//max characters that can be entered.
					//additional application state
					state_code{0},
					scroll_x{0},
					scroll_y{0},
					line{0}
				;

				std::string 
					name{},
					label{},
					text_value{},
					follows{}
				;

				unit_type_screen_element() = default;

				unit_type_screen_element& 
				operator=(const unit_type_screen_element&) = default;

				template<typename T>
				T get_x()
				{
					return static_cast<T>(x);
				}

				template<typename T>
				T get_y()
				{
					return static_cast<T>(y);
				}

				template<typename T>
				T get_w()
				{
					return static_cast<T>(w);
				}

				template<typename T>
				T get_h()
				{
					return static_cast<T>(h);
				}

				template<typename T>
				T get_input_limit()
				{
					return static_cast<T>(input_limit);
				}

				unit_type_area 
				get_area()
				{
					const type_size_ar 
						magnitude = 0
					;

					unit_type_area 
						ar(x, y, w, h, magnitude)
					;

					return ar;
				}
				
				void set_area(const unit_type_area& ar)
				{
					x = ar.x;
					y = ar.y;
					w = ar.w;
					h = ar.h;

					return;
				}
			};

			using list_type_screen_element = std::vector<unit_type_screen_element>;

			struct unit_type_screen
			{
				list_type_screen_element 
					screen_elements = {}
				;

				bool 
					changed = false
				;

				std::string name{},
					label{}
				;
			};

			using list_type_screen = std::vector<unit_type_screen>;
			using list_type_ptr_screen = std::shared_ptr<list_type_screen>;

			struct activity
			{
				bool
					window_opening = false,
					window_closing = false,
					window_esc_key_pressed = false,
					visual_clicked = false
				;

				std::string 
					screen_name{},
					screen_element_name{},
					forwarded_event_tag{},
					data{}
				;
			};

			using callback_activity_processor = std::function<void(const activity&, activity&)>;

			struct unit_type_screen_element_in_screen_name_pair
			{
				std::string
					screen_name{},
					screen_element_name{}
				;

				unit_type_screen_element_in_screen_name_pair(const std::string& screen_name_value, const std::string& screen_element_name_value)
				{
					screen_name = screen_name_value;
					screen_element_name = screen_element_name_value;
				}
			};

			visual_rgb_type create_rgb_color(std::initializer_list<int>& color_components);

			void copy_screen_elements(const unit_type_screen& src_screen, list_type_ptr_screen dest_screens);
			void insert_screen_elements(const unit_type_screen_element_in_screen_name_pair& parent_child_names, list_type_ptr_screen screens, list_type_screen_element new_elements);

			bool is_screen_changed (const unit_type_screen value);

			long count_changed_screens (list_type_ptr_screen screens);

			unit_type_screen* 
			find_changed_screens (list_type_ptr_screen screens);

			unit_type_screen* 
			find_screen_by_name (list_type_ptr_screen screens, const std::string& name);

			unit_type_screen_element* 
			find_screen_element_by_name(list_type_ptr_screen screens, const std::string& screen_name, const std::string& screen_element_name);

			unit_type_screen_element* 
			find_screen_element_by_name(list_type_screen_element& screen_elements, const std::string& screen_element_name);

			void axis_modify_x_center_align(const unit_type_area& src, unit_type_area& rel);
			void axis_modify_x_right_align(const unit_type_area& src, unit_type_area& rel);
			void axis_modify_y_center_align(const unit_type_area& src, unit_type_area& rel);
			void axis_modify_y_space_align(const unit_type_area& src, unit_type_area& rel);
			void axis_modify_y_top_align(const unit_type_area& src, unit_type_area& rel);
			void axis_modify_y_bottom_align(const unit_type_area& src, unit_type_area& rel);

			void arrange_elements(unit_type_screen* screen, const type_size_ar screen_width, const type_size_ar screen_height);

			//Diagnostic purposes.
			void print_defined_screens(list_type_ptr_screen dest_screens);
		}
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

