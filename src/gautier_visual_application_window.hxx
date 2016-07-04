#ifndef __gautier_visual_application_application_window__
#define __gautier_visual_application_application_window__

#include <memory>
#include <tuple>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include "gautier_visual_model.hxx"

namespace gautier
{
	namespace visual_application
	{
		namespace ns_visual_model = gautier::visual_application::visual_model;

		//caller driven initialization
		ns_visual_model::window_config_type 
		create_config(
			ns_visual_model::window_size_type window_size, 
			std::string title, 
			ns_visual_model::visual_rgb_type background_color
		);

		ns_visual_model::list_type_ptr_screen 
		create_screen_list();

		void register_process_request_callback(
			ns_visual_model::callback_activity_processor process_request_callback
		);

		//Running the visual engine
		void activate();
		int execute();

		//Mapping application to visual
		const ns_visual_model::activity 
		get_past_activity();

		//Abstract methods to modify visuals.
		void reset_screen_element(const std::string& screen_element_name);

		void set_screen_element(const std::string& screen_element_name, const std::string& value);
		void set_screen_element(const std::string& screen_element_name, const std::map<std::string, std::string>& data_items);

		ns_visual_model::activity create_activity();
	}
}
#endif
//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

