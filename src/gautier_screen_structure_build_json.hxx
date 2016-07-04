#ifndef __gautier_visual_application_visual_model_screen_build_json__
#define __gautier_visual_application_visual_model_screen_build_json__

#include <functional>
#include "gautier_visual_model.hxx"

namespace gautier
{
	namespace visual_application
	{
		namespace visual_model
		{
			namespace screen_build_json
			{
				gautier::visual_application::visual_model::unit_type_screen 
				create_screen(const std::function<const std::string()> layout_data_getter);
			}
		}
	}
}

#endif
