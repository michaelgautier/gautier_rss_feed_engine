#include <algorithm>
#include <iostream>

#include "gautier_visual_model.hxx"
#include "gautier_screen_structure_build.hxx"
#include "gautier_screen_structure_build_json.hxx"
#include "gautier_diagnostics.hxx"

namespace ns_visual_model = gautier::visual_application::visual_model;
namespace ns_visual_build = gautier::visual_application::visual_model::screen_build;
namespace ns_visual_build_json = gautier::visual_application::visual_model::screen_build_json;
namespace ns_diag = gautier::program::diagnostics;

//Module level implementation.

void 
ns_visual_build::get_all_screens(list_type_ptr_screen dest_screens, list_type_screen_layout_data_getter screen_layout_calls)
{
	ns_diag::write_cout_prog_unit(__FILE__, __LINE__, __func__);

	for(auto screen_layout_text_get : screen_layout_calls)
	{
		const unit_type_screen
		src_screen = ns_visual_build_json::create_screen(screen_layout_text_get);

		ns_visual_model::copy_screen_elements(src_screen, dest_screens);
	};

	return;
}

