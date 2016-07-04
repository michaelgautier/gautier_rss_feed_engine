#ifndef __gautier_visual_application_visual_model_screen_build__
#define __gautier_visual_application_visual_model_screen_build__

#include <vector>

#include "gautier_visual_model.hxx"

namespace gautier
{
	namespace visual_application
	{
		namespace visual_model
		{
			namespace screen_build
			{
				using list_type_screen_layout_data_getter = std::vector<std::function<const std::string()>>;
				using list_type_ptr_screen = gautier::visual_application::visual_model::list_type_ptr_screen;

				//Translate visuals defined in text into application defined data structures.
				//Collects all screens into a top level data structure.
				void get_all_screens(list_type_ptr_screen screens, list_type_screen_layout_data_getter layout_data_getters);
			}
		}
	}
}

#endif
