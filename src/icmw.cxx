#include "icmw.hxx"
#include "gautier_rss_model.hxx"


using namespace gautier::rss::rt;

Fl_Pack* _render_target_root = nullptr;
Fl_Pack* _render_target_feed_items_root = nullptr;
Fl_Pack* _render_target_feed_items_ictriggers = nullptr;

Fl_Hold_Browser* _render_target_feed_sources = nullptr;
Fl_Hold_Browser* _render_target_feed_items = nullptr;
Fl_Help_View* _render_target_feed_item_details = nullptr;
 
Fl_Button* _ictrigger_refresh = nullptr;
	
int _LastWidth = 0;
int _LastHeight = 0;

int _feed_sources_width = 300;

std::map<std::string, gautier::rss_model::unit_type_rss_source> _rss_feed_sources;
std::map<std::string, std::vector<gautier::rss_model::unit_type_rss_item>> _rss_feed_items;

std::string _current_feed_name;

void feed_items_callback(Fl_Widget* s, void* data) {
	int rtfs_i = _render_target_feed_items->value();
	const char* feed_item_name = _render_target_feed_items->text(rtfs_i);

	if(feed_item_name)
	{
		std::vector<gautier::rss_model::unit_type_rss_item> feed_items = _rss_feed_items[_current_feed_name];

		_render_target_feed_item_details->value("");
		
		for(auto& feed_item : feed_items)
		{
			if(feed_item.title == feed_item_name)
			{
				const char* rss_details = feed_item.description.data();

				_render_target_feed_item_details->value(rss_details);

				break;
			}
		}
	}
	else
	{
		std::cout << "feed item clicked\r\n";
	}

	return;
}

void feed_sources_callback(Fl_Widget* s, void* data) {
	int rtfs_i = _render_target_feed_sources->value();
	const char* feed_source_name = _render_target_feed_sources->text(rtfs_i);

	if(feed_source_name)
	{
		_current_feed_name = feed_source_name;
		
		std::string feed_source_url = std::string(_rss_feed_sources[_current_feed_name].url);

		std::cout << feed_source_name << " @ " << feed_source_url << "\r\n";
		
		std::vector<gautier::rss_model::unit_type_rss_item> feed_items = _rss_feed_items[_current_feed_name];
		
		std::cout << "feed item count: " << feed_items.size() << "\r\n";
		
		_render_target_feed_items->clear();
		
		for(auto& feed_item : feed_items)
		{
			const char* rss_headline = feed_item.title.data();

			_render_target_feed_items->add(rss_headline);
		}
	}
	else
	{
		std::cout << "feed item clicked\r\n";
	}

	return;
}

void ictrigger_refresh_callback(Fl_Widget* s) {
	std::cout << "refesh button clicked\r\n";

	return;
}

void resize_rss_feed_rts(int workarea_w, int workarea_h) {
	_feed_sources_width = workarea_w/5;

	_render_target_feed_sources->size(_feed_sources_width,workarea_h);

	int fs_root_w = workarea_w - _feed_sources_width;
	int fi_details_h = workarea_h/3;
	int fi_items_h = workarea_h - fi_details_h;

	_render_target_feed_items_root->size(fs_root_w, workarea_h);
	_render_target_feed_items->size(fs_root_w, fi_items_h);
	_render_target_feed_item_details->size(fs_root_w, fi_details_h);

	return;
}

void resize_workarea(int w, int h) {
	resize_rss_feed_rts(w, h);

	return;
}

int icmw::render() {
	int dv = 2, ht = 8, xy = 0;

	int workarea_w = Fl::w();
	int workarea_h = Fl::h();

	resize(xy, xy, workarea_w, workarea_h);

	size_range(480, 320, workarea_w, workarea_h);

	label("Gautier RSS");

	_render_target_root = new Fl_Pack(xy, xy, dv, workarea_h);
	_render_target_root->type(Fl_Pack::HORIZONTAL);
	_render_target_root->spacing(1);

	_render_target_feed_sources = new Fl_Hold_Browser(xy, xy, dv, dv);
	_render_target_root->add(_render_target_feed_sources);
	_render_target_feed_sources->callback(feed_sources_callback);

	_render_target_feed_items_root = new Fl_Pack(xy, xy, dv, dv);
	_render_target_feed_items_root->type(Fl_Pack::VERTICAL);
	_render_target_feed_items_root->spacing(2);

	Fl_Group::current(_render_target_feed_items_root);

	_render_target_feed_items = new Fl_Hold_Browser(xy, xy, dv, dv);
	_render_target_feed_items->callback(feed_items_callback);

	_render_target_feed_items_ictriggers = new Fl_Pack(xy, xy, dv, 28);
	_render_target_feed_items_ictriggers->type(Fl_Pack::HORIZONTAL);
	_render_target_feed_items_ictriggers->spacing(1);	

	Fl_Group::current(_render_target_feed_items_ictriggers);

	_ictrigger_refresh = new Fl_Button(xy, xy, 120, 28, "Refresh");
	_ictrigger_refresh->callback(ictrigger_refresh_callback);

	Fl_Group::current(_render_target_feed_items_root);

	_render_target_feed_item_details = new Fl_Help_View(xy, xy, dv, dv);

	resize_workarea(workarea_w, workarea_h);

	//This part needs to be in a separate thread or timer or something
	std::string rss_feeds_sources_file_name = "feeds.txt";

	gautier::rss_model::load_feeds_source_list(rss_feeds_sources_file_name, _rss_feed_sources);

	for(const auto& rss_feed_source : _rss_feed_sources)
	{
		const char* feed_source_name = rss_feed_source.first.data();
				
		_render_target_feed_sources->add(feed_source_name);
	}

	if(!_rss_feed_sources.empty())
	{
		gautier::rss_model::collect_feeds(_rss_feed_sources);
		
		gautier::rss_model::load_feeds(_rss_feed_items);
	}
	//end multi-threaded part


	end();
	show();

	while(Fl::check()) {
		int _ThisW = this->w();
		int _ThisH = this->h();

		if(_ThisW != _LastWidth || _ThisH != _LastHeight) {
			resize_workarea(_ThisW, _ThisH);

			_LastWidth = _ThisW;
			_LastHeight = _ThisH;
			
			redraw();
		}		
	}

	delete _render_target_feed_item_details;
	delete _ictrigger_refresh;
	delete _render_target_feed_items_ictriggers;
	delete _render_target_feed_items;
	delete _render_target_feed_items_root;
	delete _render_target_feed_sources;
	delete _render_target_root;

	return 0;
}

/*Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.*/

