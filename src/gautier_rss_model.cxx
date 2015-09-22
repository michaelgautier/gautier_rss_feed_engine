#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <sqlite3.h>

#include "gautier_diagnostics.hxx"
#include "gautier_rss_model.hxx"

//Michael Gautier
//On 9/10/2015			Started this.
//On 9/20/2015 11:11 PM.	Overall solution finished.
//Applied various tweaks between 10am - 1:30pm, 9/21/2015.
//On 9/21/2015 02:08 PM.	Casual run of Valgrind reports no memory leaks or allocation errors.
namespace ns_grss_model = gautier::rss_model;

//Module level types and type aliases.
enum parameter_data_type
{
	none,
	text,
	integer
};

using unit_type_query_value = std::map<std::string, std::string>;
using unit_type_query_values = std::vector<unit_type_query_value>;
using unit_type_parameter_binding = std::tuple<std::string, std::string, parameter_data_type>;

//Implementation, module level variables.

static constexpr bool 
	_sql_trace_enabled = false,
	_rows_affected_output_enabled = false,
	_issued_sql_output_enabled = false,
	_query_values_translator_output_enabled = false,
	_invalid_pointers_output_enabled = false,
	_xerces_string_translate_output_enabled = false,
	_xml_parse_status_output_enabled = false
;

static bool 
	//Always init this value to false. 
	//Dynamically set by trace functions.
	_sql_trace_active = false
;

static const char 
	_comment_marker = '#',
	_feed_config_line_sep = '\t'
;

static const int 
	_list_reserve_size = 200
;

static const std::string 
	_element_name_item = "item",
	_empty_str = "",
	_rss_database_name = "rss_feeds_info.db"
;

static const std::vector<std::string> 
	_element_names = {"title", "link", "description", "pubdate"},
	_table_names = {"rss_feed_source", "rss_feed_data", "rss_feed_data_staging"}
;

static const xercesc::DOMNode::NodeType 
	_node_type_element = xercesc::DOMNode::NodeType::ELEMENT_NODE
;

static std::string 
	_begin_transaction_sql_text = "BEGIN IMMEDIATE TRANSACTION",
	_commit_transaction_sql_text = "COMMIT TRANSACTION"
;

static std::vector<unit_type_parameter_binding> 
	_empty_param_set = {
		unit_type_parameter_binding("", "", parameter_data_type::none)
	}
;

//Implementation, top-level logic
//Largely SQL API dependent.
static void filter_feeds_source(const ns_grss_model::unit_type_list_rss_source& feed_sources, ns_grss_model::unit_type_list_rss_source& final_feed_sources);
static void save_feeds(const ns_grss_model::unit_type_list_rss& rss_feeds);
static void load_saved_feeds(ns_grss_model::unit_type_list_rss& rss_feeds);

//Implementation, supporting logic.
//XML API dependent
//Takes a source of data, defined in the XML format, using the RSS 1.0 schema
//	and converts it to various application defined data structures.
//*These output data structures drive the entire rss engine.
//*	unit_type_list_rss and unit_type_list_rss_item are the main data structures.
static void collect_feed_items_from_rss(const ns_grss_model::unit_type_list_rss_source& feed_sources, ns_grss_model::unit_type_list_rss& rss_feeds);
static void collect_feed_items(const xercesc::DOMElement* xml_element, ns_grss_model::unit_type_list_rss_item& feed_items);
static void collect_feed_items(const xercesc::DOMNode* xml_element, ns_grss_model::unit_type_list_rss_item& feed_items);
static bool is_an_approved_rss_data_name(const std::string& element_name);
static std::string get_string(const XMLCh* xstring_in);

//SQL: Database infrastructure/tables.
static bool create_database(sqlite3** db_connection);
static bool check_tables(sqlite3** db_connection);
static bool create_table(sqlite3** db_connection, const std::string& table_name);

//SQL: Queries
static unit_type_parameter_binding create_binding(const std::string name, const std::string value, const parameter_data_type parameter_type);
static std::pair<bool, int> apply_sql(sqlite3** db_connection, std::string& sql_text, std::vector<unit_type_parameter_binding>& parameter_binding_infos, std::shared_ptr<unit_type_query_values> query_values);
static int translate_sql_result(void* user_defined_data, int column_count, char** column_values, char** column_names);
static std::string get_first_string(const unit_type_query_value& row_of_data, const std::string& col_name);

//SQL: Diagnostics
static void output_data_rows(const std::shared_ptr<unit_type_query_values> query_values);

static void enable_op_sql_trace(sqlite3** db_connection);
static void trace_sql_op(void*, const char*);

static void enable_op_sql_autolog();
static void log_sql_op_event(void *pArg, int iErrCode, const char *zMsg);

static void output_op_sql_error_message(char** error_message, const int& line_number);
static void output_op_sql_error_message(sqlite3** db_connection, const int& line_number);

//Public, API.

//Take a file with name/value pairs and converts them into a 
//	a data structure by the name of unit_type_list_rss_source.
//	The collect_feed_items_from_rss function is the main function and it needs 
//		an input data structure of type, unit_type_list_rss_source.
//This function provides that output that is the input into collect_feed_items_from_rss.
//This function is optional and is one form of a solution for deriving the output.
//This function deals with a plain-text file. The file format has 
//	the name of the feed in column 1 and the rss feed url in column 2. 
//	Each column is separated by a tab.
//This version reads all lines into a data structure to be fed into collect_feed_items_from_rss function.
//Since the number of lines will not exceed a 100 or 200 lines, this approach is acceptable. 
//An alternative version that reads each line into a database table as a line is encountered
//	was considered for version 1, but is deferred until such time such an optimization is preferred.
//The current approach has more flexibility in that it does not initially depend on anything else.
//That is, the top-level functions can be adapted to data sources of any type since they are not 
//	tightly coupled to any specific data format other than a plain-text file.

void //*This function, or a function like it, has to be called first.
ns_grss_model::load_feeds_source_list(const std::string& feeds_list_file_name, ns_grss_model::unit_type_list_rss_source& feed_sources)
{
	if(!feeds_list_file_name.empty())
	{
		ns_grss_model::
		unit_type_list_rss_source tmp_feed_sources;

		std::ifstream feeds_file;
		feeds_file.open(feeds_list_file_name);

		if(feeds_file)
		{
			while(feeds_file.good() && !feeds_file.eof())
			{
				std::string line_data = _empty_str;

				std::getline(feeds_file, line_data);

				if(!line_data.empty() && line_data.front() != _comment_marker)
				{
					//Consider a validation in the future for URL and name.
					auto tab_pos = line_data.find(_feed_config_line_sep);

					if(tab_pos != std::string::npos)
					{
						std::string label = std::string(line_data, 0, tab_pos);
						std::string url = std::string(line_data, tab_pos+1, std::string::npos);

						tmp_feed_sources[label] = url;
					}
				}
			}
		}

		feed_sources = tmp_feed_sources;
	}

	return;
}

//Main logic.
//Ties together the process of pulling in rss feed data (in XML format) 
//	into a data structure named unit_type_list_rss that is used 
//	by other processes to present rss headline and web address information.
void 
ns_grss_model::collect_feeds(const ns_grss_model::unit_type_list_rss_source& feed_sources, ns_grss_model::unit_type_list_rss& rss_feeds)
{
	ns_grss_model::unit_type_list_rss_source final_feed_sources;

	filter_feeds_source(feed_sources, final_feed_sources);

	if(!final_feed_sources.empty())
	{
		collect_feed_items_from_rss(feed_sources, rss_feeds);

		save_feeds(rss_feeds);

		load_saved_feeds(rss_feeds);
	}

	return;
}

//Presents rss feed information to standard output.
//An optional, convenience function primarily for diagnostic/testing purposes 
//	but whose sequence can be adapted to other file based output processes.
//One of those processes can involve converting the rss feed information into 
//	HTML file format.
//HTML output is not designed into this version, but would be a quick way 
//	to put feeds into a format that can be immediately used in a web browser.
void 
ns_grss_model::output_feeds(const ns_grss_model::unit_type_list_rss& rss_feeds)
{
	const std::string heading_line = "***********************************************";

	std::stringstream ostr;

	//each feed.
	for(const auto& named_list : rss_feeds)
	{
		const std::string& list_name = named_list.first;

		ostr << heading_line << "\n";
		ostr << "\t\t" << list_name << "\n";
		ostr << heading_line << "\n";

		ns_grss_model::
		unit_type_list_rss_item feed_list = named_list.second;

		//Each value is an anonymous item that is a list of name/value pairs
		for(unit_type_list_rss_item_value& feed_item : feed_list)
		{
			ostr << "------ details ----------------\n";

			//Individual value from an item group (description, link, date, etc)
			for(const std::string data_name : _element_names)
			{
				const std::string data_value = feed_item[data_name];

				ostr << data_name << "\n\t" << data_value << "\n\n";
			}
		}
	}

	std::cout << ostr.str();

	return;
}

//Private, module level implementation.
//Encapsulates all the logic necessary to determine which rss sources to pull fresh.
//Uses an embedded database to conduct the filtering and incorporate/extract data.

//The main purpose is to avoid unnecessary repeat calls to an rss provider.
//There are simpler and easier ways to accomplish this other than the approach chosen.
//The reason this approach was chosen, boilerplate and all, was due to the fact
//that individual rss feeds data would not be kept in separate files.
//They could, and they were for testing purposes, but the final solution is to have no 
//external parts other than network calls and a single database file. Self-contained program.
//Consolidating them in an embedded database is more useful but that comes with a complexity cost.
static void 
filter_feeds_source(const ns_grss_model::unit_type_list_rss_source& feed_sources, ns_grss_model::unit_type_list_rss_source& final_feed_sources)
{
	enable_op_sql_autolog();

	sqlite3* db_connection = nullptr;

	create_database(&db_connection);

	if(db_connection)
	{
		bool tables_exist = check_tables(&db_connection);

		if(tables_exist)
		{
			//COUNT FEED SOURCES.
			int initial_count_feed_sources = 0;
			{
				char* error_message = 0;
				auto sql_query_exec_result = SQLITE_BUSY;

				unit_type_query_values* query_values = 
				new unit_type_query_values;

				{
					std::string 
					sql_text = 
					"SELECT \
						 COUNT(*) AS row_count \
					FROM rss_feed_source; \
					";

					sql_query_exec_result = 
					sqlite3_exec(db_connection, sql_text.data(), translate_sql_result, query_values, &error_message);
				}

				if(sql_query_exec_result == SQLITE_OK)
				{
					if(query_values)
					{
						//APPLY NEW NAMES TO OLD.
						for(unit_type_query_value& row_of_data : *query_values)
						{
							initial_count_feed_sources = std::stoi(row_of_data["row_count"]);
						}
					
						delete query_values;
					}
				}
				else
				{
					output_op_sql_error_message(&error_message, __LINE__);
				}
			}

			//IMPORT RSS FEED SOURCES.
			{
				apply_sql(&db_connection, _begin_transaction_sql_text, _empty_param_set, nullptr);

				std::string 
				sql_text = 
				"INSERT INTO rss_feed_source(name, url) VALUES (trim(@name), trim(@url));";

				for(const auto& feed_source : feed_sources)
				{
					std::vector<unit_type_parameter_binding> parameter_values = 
					{
						create_binding("@name", feed_source.first, parameter_data_type::text),
						create_binding("@url", feed_source.second, parameter_data_type::text)
					};

					apply_sql(&db_connection, sql_text, parameter_values, nullptr);
				}

				apply_sql(&db_connection, _commit_transaction_sql_text, _empty_param_set, nullptr);
			}

			//COLLECT RSS FEED NAME CHANGES.
			{
				char* error_message = 0;
				auto sql_query_exec_result = SQLITE_BUSY;

				unit_type_query_values* query_values = 
				new unit_type_query_values;

				{
					std::string 
					sql_text = 
					"SELECT \
						 dest.id AS dest_id, \
						 dest.name AS dest_name, \
						 src.id AS src_id, \
						 src.name AS src_name \
					FROM (SELECT id, name, url FROM rss_feed_source WHERE type_code = 0) AS dest INNER JOIN \
					(SELECT id, name, url FROM rss_feed_source WHERE type_code = 1) AS src ON dest.url = src.url \
					COLLATE NOCASE; \
					";

					sql_query_exec_result = 
					sqlite3_exec(db_connection, sql_text.data(), translate_sql_result, query_values, &error_message);
				}

				//RECONCILE OLD NAMES WITH NEW.
				if(sql_query_exec_result == SQLITE_OK)
				{
					if(query_values)
					{
						apply_sql(&db_connection, _begin_transaction_sql_text, _empty_param_set, nullptr);

						//APPLY NEW NAMES TO OLD.
						for(const unit_type_query_value row_of_data : *query_values)
						{
							//If the input names changed,
							//but the url stayed the same, update the names to match.
							std::string sql_text = 
							"UPDATE rss_feed_source SET name = @name WHERE id = @id;";

							std::vector<unit_type_parameter_binding> parameter_values = 
							{
								create_binding("@name", get_first_string(row_of_data, "src_name"), parameter_data_type::text),
								create_binding("@id", get_first_string(row_of_data, "dest_id"), parameter_data_type::integer)
							};

							apply_sql(&db_connection, sql_text, parameter_values, nullptr);
						}

						//REMOVE DUPLICATE RSS FEED SOURCES.
						//Once the names are synched, 
						//remove the source update data.
						for(const unit_type_query_value row_of_data : *query_values)
						{
							std::string sql_text = 
							"DELETE FROM rss_feed_source WHERE id = @id;";

							std::vector<unit_type_parameter_binding> parameter_values = 
							{
								create_binding("@id", get_first_string(row_of_data, "src_id"), parameter_data_type::integer)
							};

							apply_sql(&db_connection, sql_text, parameter_values, nullptr);
						}

						delete query_values;

						apply_sql(&db_connection, _commit_transaction_sql_text, _empty_param_set, nullptr);
					}
				}
				else
				{
					output_op_sql_error_message(&error_message, __LINE__);
				}
			}

			//ACCEPT REMAINING RSS FEED SOURCES.
			{
				apply_sql(&db_connection, _begin_transaction_sql_text, _empty_param_set, nullptr);

				//Any remaining entries will be accepted.
				std::string sql_text = 
				"UPDATE rss_feed_source set type_code = 0 WHERE type_code = 1";

				apply_sql(&db_connection, sql_text, _empty_param_set, nullptr);

				apply_sql(&db_connection, _commit_transaction_sql_text, _empty_param_set, nullptr);
			}

			//When a database is first created new (or is re-created)
			//	there are no entries except the ones initially used 
			//	to populate it. Therefore, it makes sense to return 
			//	all data in which subsequent runs will use normal expiration date logic.
			if(initial_count_feed_sources > 0)
			{
				//***
				//	MAIN SQL QUERY.
				//	Data output drives the main program.
				//***

				//The preceding was a prerequisite for enabling the retrieval of 
				//	feed sources based on expiration date.

				//This is the main driver for the entire program.
				//The data.
				//A possible modification is to set the 
				//expiration offset in the where clause
				//using a dynamic input to parameterize
				//the value.
				std::string sql_text = 
				"SELECT \
				 \
					id,\
					type_code,\
					entry_date,\
					name,\
					url\
				 FROM rss_feed_source \
				 WHERE \
				 (datetime(entry_date, '+1 hour')) < (datetime('now', 'localtime'));\
				";

				unit_type_query_values* query_values = 
				new unit_type_query_values;

				char* error_message = 0;

				auto sql_query_exec_result = 
				sqlite3_exec(db_connection, sql_text.data(), translate_sql_result, query_values, &error_message);

				if(sql_query_exec_result == SQLITE_OK)
				{
					if(query_values)
					{
						for(auto& row_of_data : *query_values)
						{
							std::string name = row_of_data["name"];
							std::string url = row_of_data["url"];

							final_feed_sources[name] = url;
						}

						delete query_values;
					}
				}
				else
				{
					output_op_sql_error_message(&error_message, __LINE__);
				}
			}
			else
			{
				final_feed_sources = feed_sources;
			}
		}//end of table scope

		sqlite3_close(db_connection);
	}

	return;
}

//THE TRUE TRIGGER FOR RSS DOWNLOAD.
//The idea is to update the main entry date whenever a feed is accessed over the network.
static void 
save_feeds(const ns_grss_model::unit_type_list_rss& rss_feeds)
{
	using type_feed_item_value = ns_grss_model::unit_type_list_rss_item_value;
	using type_feed_item = ns_grss_model::unit_type_list_rss_item;//vector of item value
	using type_feed = std::pair<std::string, type_feed_item>;

	sqlite3* db_connection = nullptr;

	if(!rss_feeds.empty())
	{
		create_database(&db_connection);
	}

	if(db_connection)
	{
		enable_op_sql_trace(&db_connection);

		apply_sql(&db_connection, _begin_transaction_sql_text, _empty_param_set, nullptr);

		for(const type_feed& rss_feed : rss_feeds)
		{
			int rss_feed_source_id = 0;

			const std::string rss_feed_name = rss_feed.first;
			type_feed_item feed_item = rss_feed.second;

			//GET RSS FEED DESCRIPTION RECORD.
			{
				std::string 
				sql_text = 
				"SELECT \
					 id \
				FROM rss_feed_source \
				WHERE name = @name; \
				";

				std::vector<unit_type_parameter_binding> parameter_values = 
				{
					create_binding("@name", rss_feed_name, parameter_data_type::text)
				};

				std::shared_ptr<unit_type_query_values> query_values;
				query_values.reset(new unit_type_query_values);

				apply_sql(&db_connection, sql_text, parameter_values, query_values);

				if(query_values)
				{
					for(auto& row_of_data : *query_values)
					{
						std::string id_text_value = 
						row_of_data["id"];

						if(!id_text_value.empty())
						{
							rss_feed_source_id = std::stoi(id_text_value);
						}

						break;
					}
				}

				//ABORTS THE ENTIRE OPERATION for this feed.
				//Without a source_id, there is no linkage that can be made.
				if(rss_feed_source_id == 0)
				{
					std::cout 
					<< "not adding feed " 
					<< rss_feed_name << ", see:" 
					<< __func__ 
					<< "\n";

					break;
				}
			}

			for(type_feed_item_value& values : feed_item)
			{
				//IMPORT RSS FEED DATA.
				{
					std::string 
					sql_text = 
					"INSERT INTO rss_feed_data_staging \
					(\
						rss_feed_source_id,\
						pub_date,\
						title,\
						link,\
						description\
					)\
					VALUES \
					(\
						@rss_feed_source_id,\
						@pub_date,\
						trim(@title),\
						trim(@link),\
						trim(@description)\
					)\
					;";

					std::vector<unit_type_parameter_binding> parameter_values = 
					{
						create_binding("@rss_feed_source_id", std::to_string(rss_feed_source_id), parameter_data_type::integer),
						create_binding("@pub_date", values["pubdate"], parameter_data_type::text),
						create_binding("@title", values["title"], parameter_data_type::text),
						create_binding("@link", values["link"], parameter_data_type::text),
						create_binding("@description", values["description"], parameter_data_type::text)
					};

					apply_sql(&db_connection, sql_text, parameter_values, nullptr);
				}
			}
		}

		apply_sql(&db_connection, _commit_transaction_sql_text, _empty_param_set, nullptr);

		//Transfers eligible feeds data entries from staging to active.
		//The staging data remains in place for diagnostic purposes.
		//Older entries will be purged from both tables.
		{
			apply_sql(&db_connection, _begin_transaction_sql_text, _empty_param_set, nullptr);

			std::string 
			sql_text = 
			"INSERT INTO rss_feed_data ( \
				 rss_feed_source_id, \
				 pub_date, \
				 title, \
				 link, \
				 description \
			) \
			SELECT \
				 rss_feed_source_id, \
				 pub_date, \
				 title, \
				 link, \
				 description \
			FROM rss_feed_data_staging \
			WHERE link NOT IN ( \
				SELECT \
					 link \
				FROM rss_feed_data \
				GROUP BY link \
			) GROUP BY \
				 rss_feed_source_id, \
				 pub_date, \
				 title, \
				 link, \
				 description \
			ORDER BY \
				 rss_feed_source_id, \
				 pub_date DESC, \
				 title \
			; \
			DELETE \
			FROM rss_feed_data_staging \
			WHERE (datetime(entry_date, '+8 hour')) < (datetime('now', 'localtime'));\
			DELETE \
			FROM rss_feed_data \
			WHERE (datetime(entry_date, '+1 month')) < (datetime('now', 'localtime'));\
			";

			apply_sql(&db_connection, sql_text, _empty_param_set, nullptr);

			apply_sql(&db_connection, _commit_transaction_sql_text, _empty_param_set, nullptr);
		}

		sqlite3_close(db_connection);
	}

	return;
}

static void 
load_saved_feeds(ns_grss_model::unit_type_list_rss& rss_feeds)
{
	ns_grss_model::unit_type_list_rss 
	tmp_rss_feeds;

	if(!rss_feeds.empty())
	{
		rss_feeds.clear();
	}

	sqlite3* db_connection = nullptr;

	create_database(&db_connection);

	if(db_connection)
	{
		//optimization
		//preallocate feed items in contiguous groups.
		{
			std::string sql_text = 
			"SELECT \
				fs.name, \
				COUNT(fd.id) AS total_sub_items \
			FROM rss_feed_source AS fs INNER JOIN \
			rss_feed_data AS fd ON fs.id = fd.rss_feed_source_id \
			GROUP BY fs.name \
			ORDER BY fs.name;\
			";

			unit_type_query_values* query_values = 
			new unit_type_query_values;

			char* error_message = 0;

			const auto sqlite_result = 
			sqlite3_exec(db_connection, sql_text.data(), translate_sql_result, query_values, &error_message);

			if(sqlite_result == SQLITE_OK)
			{
				if(query_values)
				{
					for(auto& row_of_data : *query_values)
					{
						const std::string feed_name = row_of_data["name"];
						const int item_count = std::stoi(row_of_data["total_sub_items"]);

						auto feed_items = &(tmp_rss_feeds[feed_name]);

						feed_items->reserve(item_count);
					}

					delete query_values;
				}
			}
			else
			{
				output_op_sql_error_message(&error_message, __LINE__);
			}
		}

		//load the feed detail.
		{
			std::string sql_text = 
			"SELECT \
				fs.name AS feed_name, \
				fd.pub_date, \
				fd.title, \
				fd.link, \
				fd.description \
			FROM rss_feed_source AS fs INNER JOIN \
			rss_feed_data AS fd ON fs.id = fd.rss_feed_source_id \
			ORDER BY \
				 fs.name, \
				 fd.pub_date, \
				 fd.title;\
			";

			unit_type_query_values* query_values = 
			new unit_type_query_values;

			char* error_message = 0;

			const auto sqlite_result = 
			sqlite3_exec(db_connection, sql_text.data(), translate_sql_result, query_values, &error_message);

			if(sqlite_result == SQLITE_OK)
			{
				if(query_values)
				{
					for(auto& row_of_data : *query_values)
					{
						std::string feed_name = row_of_data["feed_name"];
						std::string pub_date = row_of_data["pub_date"];
						std::string title = row_of_data["title"];
						std::string link = row_of_data["link"];
						std::string description = row_of_data["description"];

						ns_grss_model::
						unit_type_list_rss_item_value feed_item = 
						{
							{"pubdate", pub_date},
							{"title", title},
							{"link", link},
							{"description", description}
						};

						tmp_rss_feeds[feed_name].push_back(feed_item);
					}

					delete query_values;
				}
			}
			else
			{
				output_op_sql_error_message(&error_message, __LINE__);
			}
		}
	}

	rss_feeds = tmp_rss_feeds;

	return;
}

//Retrieves rss data at a given url, decodes the XML into a data structure named, unit_type_list_rss.
//Retrieval logic is done by Apache Xerces which will pull from a file location or web address.
//After retrieval, xml represented as DOM objects.
//The DOM is traversed with relevant DOM items converted into entries in a unit_type_list_rss data structure.
static void 
collect_feed_items_from_rss(const ns_grss_model::unit_type_list_rss_source& feed_sources, ns_grss_model::unit_type_list_rss& rss_feeds)
{
	xercesc::XMLPlatformUtils::Initialize();
	{
		for(auto& feed_source : feed_sources)
		{
			std::shared_ptr<xercesc::XercesDOMParser> rss_feed_data_parser;
			rss_feed_data_parser.reset(new xercesc::XercesDOMParser);

			rss_feed_data_parser->setIncludeIgnorableWhitespace(false);
			rss_feed_data_parser->setLoadSchema(false);
			rss_feed_data_parser->setLoadExternalDTD(false);
			rss_feed_data_parser->setCreateCommentNodes(false);

			if(_xml_parse_status_output_enabled)
			{
				std::cout << "parsing: " << feed_source.first << ".\n";
			}

			rss_feed_data_parser->parse(feed_source.second.data());

			//The deleter from the parser will ensure subsequent objects are deallocated.
			//Confirmed this with Valgrind.
			if(rss_feed_data_parser)
			{
				xercesc::DOMDocument* rss_feed_xml_dom = rss_feed_data_parser->getDocument();

				if(rss_feed_xml_dom)
				{
					xercesc::DOMElement* rss_feed_xml_root = rss_feed_xml_dom->getDocumentElement();

					if(rss_feed_xml_root)
					{
						{
							ns_grss_model::
							unit_type_list_rss_item rss_feed_values;
							rss_feed_values.reserve(_list_reserve_size);

							rss_feeds[feed_source.first] = rss_feed_values;
						}

						if(_xml_parse_status_output_enabled)
						{
							std::cout << "processing: " << feed_source.first << ".\n";
						}

						collect_feed_items(rss_feed_xml_root, rss_feeds[feed_source.first]);

						if(_xml_parse_status_output_enabled)
						{
							std::cout << "processing done.\n";
						}
					}

				}
			}

			if(_xml_parse_status_output_enabled)
			{
				std::cout << "parse completed.\n";
			}
		}
	}
	xercesc::XMLPlatformUtils::Terminate();

	return;
}

//See the information for Overload #2 for this function.
//This is an adapter function that transitions a recursive call from 
//	an interface that uses an XMLElement type to one that uses a more 
//	generic DOMNode type. This adaptation is encapsulated here rather than 
//	leave that as an exercise immediately following XML parsing.
//Overload #1. This will normally be called first when using Xerces in the way
//	Xerces is used in this module.
static void 
collect_feed_items(const xercesc::DOMElement* xml_element, ns_grss_model::unit_type_list_rss_item& feed_items)
{
	const xercesc::DOMNodeList* nodes = xml_element->getChildNodes();

	const XMLSize_t total_nodes = nodes->getLength();

	for(XMLSize_t node_n = 0; node_n < total_nodes; node_n++)
	{
		xercesc::DOMNode* current_node = nodes->item(node_n);

		collect_feed_items(current_node, feed_items);
	}

	return;
}

//Iterates through XML Elements represented as generic DOM Nodes.
//The DOM Nodes interface was chosen since it is generally the same across
//	many XML API. This allows you to switch the code to another 
//	XML API more easily if that is necessary. The key function calls 
//	for node data access is some form of the following:
//		get the element name of the node	local name or unqualified name
//		get the text data an element node	node text or node value
//		get the sub nodes of an element node	child nodes or children
//This function is the main function in terms of data retrieval.
//Overload #2.
static void 
collect_feed_items(const xercesc::DOMNode* xml_element, ns_grss_model::unit_type_list_rss_item& feed_items)
{
	if(xml_element->getNodeType() == _node_type_element)
	{
		std::string parent_local_name;
		{
			XMLCh* xstr = xercesc::XMLString::replicate(xml_element->getNodeName());

			parent_local_name = get_string(xstr);

			xercesc::XMLString::release(&xstr);
		}

		const xercesc::DOMNodeList* nodes = xml_element->getChildNodes();

		const XMLSize_t total_nodes = nodes->getLength();

		for(XMLSize_t node_n = 0; node_n < total_nodes; node_n++)
		{
			const xercesc::DOMNode* current_node = nodes->item(node_n);

			if(current_node)
			{
				if(current_node->getNodeType() != _node_type_element)
				{
					continue;
				}

				std::string current_local_name;
				{
					XMLCh* xstr = xercesc::XMLString::replicate(current_node->getNodeName());

					current_local_name = get_string(xstr);

					xercesc::XMLString::release(&xstr);
				}

				if(parent_local_name == _element_name_item)
				{
					if(is_an_approved_rss_data_name(current_local_name))
					{
						std::string node_data;
						{
							XMLCh* xstr = xercesc::XMLString::replicate(current_node->getTextContent());

							node_data = get_string(xstr);

							xercesc::XMLString::release(&xstr);
						}

						ns_grss_model::
						unit_type_list_rss_item_value& feed_item = feed_items.back();

						auto switch_letter_case = [](const char& in_char){return std::tolower(in_char);};

						std::transform(current_local_name.begin(), current_local_name.end(), current_local_name.begin(), switch_letter_case);

						feed_item[current_local_name] = node_data;
					}
				}
				else
				{
					if(current_local_name == _element_name_item)
					{
						feed_items.push_back(ns_grss_model::unit_type_list_rss_item_value());
					}

					collect_feed_items(current_node, feed_items);
				}
			}
			else
			{
				if(_invalid_pointers_output_enabled)
				{
					std::cout << "node not valid\n";
				}
			}
		}
	}

	return;
}

//Defines interest in specific element names from the input XML data.
//This confirms that an element name exists in the list of approved
//	element names. Other code uses this to determine if data 
//	from an element should be retrieved.
static bool 
is_an_approved_rss_data_name(const std::string& element_name)
{
	bool match_found = false;

	std::string data_name = element_name;

	if(!element_name.empty())
	{
		auto switch_letter_case = [](const char& in_char){return std::tolower(in_char);};

		std::transform(data_name.begin(), data_name.end(), data_name.begin(), switch_letter_case);
	}

	auto found = 
	std::find(_element_names.cbegin(), _element_names.cend(), data_name);

	match_found = (found != _element_names.cend());

	return match_found;
}

//Encapsulates all logic for converting from a Xerces String to 
// a C++ Standard Library String, std::string in the native code page.
//*When all else is functioning, this is the most critical function in the engine.
//Oddly enough, it is only necessary with XML API that require code page translation.
//Since translation can be a granular process, this function creates a higher 
//	abstraction level that contains all processing to one area.
//The function is not very extensive, but in testing of 7,000 inputs, it works
//	to a highly acceptable level.
//This function took the most time to develop. Any potential issues noted in the function comments 
//	may be addressed by compiling with the ICU library.
//I did not test with that configuration as I was striving for the most functionality 
//	with the least amount of additional dependencies to maximize portability.
//The function will work in situations where ICU is not available and system resource is constrained.
//If you compile the Xerces API with ICU, when that is an option, 
//	the documentation does imply it will work much better.
static std::string 
get_string(const XMLCh* xstring_in)
{
	std::string value = _empty_str;

	char empty_char = *(_empty_str.data());

	value = {&empty_char};

	if(!xstring_in && _invalid_pointers_output_enabled)
	{
		std::cout << "xerces string invalid.\n";
	}
	//Documentation on the Xerces API states that XMLString is not a publicly supported
	//	part of Xerces but that they are working towards making it publicly supported.
	//At present, it means that the following method to translate a string is 
	//	not the formally approved method, but it has the benefit of being
	//	a simple and portable interface.
	//Xerces provides additional guarantees if you link it to ICU, but I decided 
	//	to forgo that for now since the capabilities of ICU are not desired 
	//	at this time and I wanted to achieve a minimally viable solution that 
	//	can function in more constrained situations where ICU may be unavailable
	//	and to allow swapping out APIs that may or may not support such extension.
	else
	{
		char* transcoded_str = xercesc::XMLString::transcode(xstring_in);
		{
			const char* str = transcoded_str;

			value = {str};
		}

		xercesc::XMLString::release(&transcoded_str);
	}

	//Fallback approach.
	//Custom defined by Michael Gautier, 9/17/2015.
	//Based on information from: 
	//The C++ Standard Library, 2nd Edition by Nicolai Josuttis, Chapter 16.
	//Recovers strings not translated by Xerces.
	//On a sample run across 19 rss feeds involving 7,506 string conversions, 
	//	this method translated 44 string unapproachable by the Xerces method above.
	//Known flaw: Not all special characters are handled by this method.
	if(value.empty())
	{
		if(_xerces_string_translate_output_enabled)
		{
			std::cout << "second decode attempt.\n";
		}

		xercesc::TranscodeToStr translator(xstring_in, "UTF-8");

		XMLSize_t translated_xstr_size = translator.length();
		const XMLByte* translated_xstr = translator.str();

		value.clear();
		value.reserve(translated_xstr_size);

		for(XMLSize_t index = 0; index < translated_xstr_size; index++)
		{
			XMLByte src_data = *(translated_xstr+index);

			const wchar_t wdata = std::char_traits<wchar_t>::to_char_type(src_data);

			auto data = std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(wdata, ' ');//substitute with space so that absent chars do not result in bunched words.

			const char cdata = data;

			value.push_back(cdata);
		}
	}

	return value;
}

//SQLite3 is not defined in C++. It is defined in C and I used it that way.
//The implemented functions that use SQLite3 will have a slight C language orientation.
//Effort was applied to encapsulate various functions to produce a useful abstraction.
//This also applies to the implementation of the functions: 
//	filter_feeds_source, save_feeds, and load_saved_feeds; that I defined to use SQLite3.
//I decided not to use predefined wrapper libraries that sit a C++ interface atop SQLite3 
//	since the use of SQLite3 in this engine is not extensive. 
//In terms of the feeds engine, use of SQLite3 is currently limited to this module.
//It is used to produce an output data structure and maintain persistent data storage that can be 
//	conveniently queried in ways that streamline resusitation of stored data into an application 
//	level data structure. The primary output is a data structure of type unit_type_list_rss.

//SQL: Database infrastructure/tables.

//The following functions under this section deals with supporting database structures for the rss engine.
//Make a database file if one does not exist.
//Not currently a halting error if this fails. 
//Rather, the process fails silently if a database cannot be made available.
static bool 
create_database(sqlite3** db_connection)
{
	bool success = false;

	auto open_result = sqlite3_open_v2(_rss_database_name.data(), &(*db_connection), SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

	if(open_result == SQLITE_OK && db_connection)
	{
		success = true;
	}
	else
	{
		std::cout 
		<< "unable to open database.\n";
	}

	return success;
}

//Examines the database for the existence of tables.
//Uses an aggregate function to determine if a table exists.
//If the table does not exist, dispatches a call to create_table, to define the table in the database.
static bool 
check_tables(sqlite3** db_connection)
{
	bool success = false;

	if(db_connection)
	{
		const auto expected = _table_names.size();

		unsigned actual = 0;

		for(const std::string table_name : _table_names)
		{
			std::string 
			sql_text = 
			"SELECT \
			 	COUNT(*) AS row_count \
			FROM sqlite_master \
			WHERE type='table' \
			  AND name = @table_name; \
			";

			std::vector<unit_type_parameter_binding> parameter_values = 
			{
				create_binding("@table_name", table_name, parameter_data_type::text)
			};

			std::shared_ptr<unit_type_query_values> query_values;
			query_values.reset(new unit_type_query_values);

			apply_sql(db_connection, sql_text, parameter_values, query_values);

			if(query_values)
			{
				const auto row_count = std::stoul(get_first_string(query_values->front(), "row_count"));

				bool exists = (row_count > 0);

				if(exists)
				{
					actual++;//note the existence of the table.
				}
				else//not exists
				{
					const bool table_created = 
					create_table(db_connection, table_name);

					if(table_created)
					{
						actual++;//note its existence.
					}
				}
			}
		}

		success = (actual == expected);
	}

	return success;
}

//Defines tables in the relational database used by the rss engine.
static bool 
create_table(sqlite3** db_connection, const std::string& table_name)
{
	bool success = false;

	if(db_connection)
	{
		std::string create_table_statement_text = "";

		if(table_name == "rss_feed_source")
		{
			create_table_statement_text = 
			"CREATE TABLE " + table_name + "\
			(\
				id INTEGER PRIMARY KEY ASC,\
				type_code INTEGER DEFAULT 1,\
				entry_date TEXT DEFAULT (datetime(CURRENT_TIMESTAMP, 'localtime')),\
				name TEXT NOT NULL COLLATE NOCASE,\
				url TEXT NOT NULL COLLATE NOCASE\
			 );\
			 ";
		}
		else if(table_name == "rss_feed_data" || table_name == "rss_feed_data_staging")
		{
			create_table_statement_text = 
			"CREATE TABLE " + table_name + "\
			(\
				id INTEGER PRIMARY KEY ASC,\
				rss_feed_source_id INTEGER NOT NULL,\
				entry_date TEXT DEFAULT (datetime(CURRENT_TIMESTAMP, 'localtime')),\
				pub_date TEXT NOT NULL,\
				title TEXT NOT NULL COLLATE NOCASE,\
				link TEXT NOT NULL COLLATE NOCASE,\
				description TEXT NOT NULL COLLATE NOCASE\
			 );\
			 ";
		}

		if(!create_table_statement_text.empty())
		{
			char* error_message = 0;

			const auto sqlite_result = 
			sqlite3_exec(*db_connection, create_table_statement_text.data(), nullptr, nullptr, &error_message);

			if(sqlite_result == SQLITE_OK)
			{
				success = true;
			}
			else
			{
				output_op_sql_error_message(&error_message, __LINE__);
			}
		}
	}

	return success;
}

//SQL: Queries

//Links a named parameter to data value for use in a parameterized sql statement.
static unit_type_parameter_binding 
create_binding(const std::string name, const std::string value, const parameter_data_type parameter_type)
{
	return unit_type_parameter_binding(name, value, parameter_type);
}

//Execute an sql statement.
//At the same level as the sqlite3_exec function.
//The sqlite3_exec function is the preferred way for any readonly queries that do not take parameters.
//The primary use of this function is to introduce parameters to an sql statement.
//Otherwise, it produces the same output as the sqlite3_exec callback for this module, translate_sql_result.
//A noteable difference with this function versus translate_sql_result is that this function will 
//	immediately output all error messages to std out rather than return an error data structure.
//As a result, the return SQLITE result code/error code is primarily for control caller control flow.
static std::pair<bool, int> 
apply_sql(sqlite3** db_connection, std::string& sql_text, std::vector<unit_type_parameter_binding>& parameter_binding_infos, std::shared_ptr<unit_type_query_values> query_values)
{
	bool success = false;
	int row_count = -1;

	sqlite3_stmt* sql_stmt = nullptr;

	if(!db_connection || sql_text.empty())
	{
		return std::pair<bool, int>(success, row_count);
	}

	auto sqlite_prepare_result = SQLITE_BUSY;
	{
		const char* sql_char_ptr = sql_text.data();
		int sql_char_ptr_sz = -1;

		sqlite_prepare_result = 
		sqlite3_prepare_v2(*db_connection, sql_char_ptr, sql_char_ptr_sz, &sql_stmt, nullptr);
	}

	if(sqlite_prepare_result == SQLITE_OK)
	{
		int params_count = 0;

		//Binding the input values to the input sql statement.
		for(auto& bind_info : parameter_binding_infos)
		{
			params_count++;

			int param_n = params_count;

			std::string parameter_name = std::get<0>(bind_info);
			std::string parameter_text = std::get<1>(bind_info);
			parameter_data_type param_t = std::get<2>(bind_info);

			auto sqlite_result = 0;

			if(param_t == parameter_data_type::text)
			{
				const char* param_char_ptr = parameter_text.data();
				int param_char_ptr_sz = -1;

				sqlite3_bind_text(sql_stmt, param_n, param_char_ptr, param_char_ptr_sz, SQLITE_TRANSIENT);
			}
			else if(param_t == parameter_data_type::integer)
			{
				int param_value = std::stoi(parameter_text);

				sqlite3_bind_int(sql_stmt, param_n, param_value);
			}
			else
			{
				if(param_t != parameter_data_type::none)
				{
					std::cout 
					<< "parameter " << parameter_name 
					<< " data type not mapped.\n";
				}
			}

			if(sqlite_result != SQLITE_OK)
			{
				output_op_sql_error_message(db_connection, __LINE__);

				break;
			}
		}

		if(static_cast<unsigned>(params_count) != parameter_binding_infos.size())
		{
			success = false;

			std::cout 
			<< "could not bind params to: " << sql_text 
			<< "\n";

			std::cout 
			<< "total parameters : " << parameter_binding_infos.size() 
			<< "\n";

			for(auto& diag_bind_info : parameter_binding_infos)
			{
				std::cout 
				<< std::get<0>(diag_bind_info) << " | " 
				<< std::get<1>(diag_bind_info) << " | " 
				<< std::to_string(static_cast<unsigned>(std::get<2>(diag_bind_info))) 
				<< "\n";
			}
		}
		else
		{
			//Run the SQL.
			auto sqlite_result = SQLITE_BUSY;

			if(!sqlite3_stmt_readonly(sql_stmt))
			{
				sqlite_result = 
				sqlite3_step(sql_stmt);
			}
			else
			{
				do
				{
					sqlite_result = 
					sqlite3_step(sql_stmt);

					if(sqlite_result == SQLITE_ROW)
					{
						if(query_values)
						{
							int total_columns = sqlite3_column_count(sql_stmt);

							unit_type_query_value row_of_data;

							for(decltype(total_columns) col_n = 0; col_n < total_columns; col_n++)
							{
								std::string column_name, column_value;

								column_name = sqlite3_column_name(sql_stmt, col_n);
								column_value = "";

								auto column_char_seq = sqlite3_column_text(sql_stmt, col_n);
								auto column_char_bytes = sqlite3_column_bytes(sql_stmt, col_n);

								auto char_sz = sizeof(*column_char_seq);
								auto chars_n = column_char_bytes/char_sz;

								for(decltype(chars_n) data_n = 0; data_n < chars_n; data_n++)
								{
									auto data = std::char_traits<char>::to_char_type(*(column_char_seq+data_n));
								
									column_value.push_back(data);
								}

								row_of_data[column_name] = column_value;
							}

							query_values->push_back(row_of_data);
						}

						success = true;
					}
				}while(sqlite_result != SQLITE_DONE);
			}

			if(sqlite_result == SQLITE_DONE)
			{
				success = true;

				row_count = sqlite3_changes(*db_connection);
			}
			else
			{
				success = false;//undoing any success up to this point.

				output_op_sql_error_message(db_connection, __LINE__);
			}
		}
	}
	else
	{
		output_op_sql_error_message(db_connection, __LINE__);
	}

	//diagnostic
	if(_issued_sql_output_enabled)
	{
		auto sql_statement_text = sqlite3_sql(sql_stmt);

		std::cout << "\t\t ran " 
		<< sql_statement_text;

		if(success)
		{
			std::cout 
			<< ". the result was good. " ;

			if(_rows_affected_output_enabled)
			{
				std::cout << row_count << " rows affected. ";
			}
		}
		else
		{
			std::cout << ". failed. ";
		}

		std::cout << "\n";
	}
	else
	{
		if(_rows_affected_output_enabled)
		{
			std::cout 
			<< row_count
			<< " rows affected.\n";
		}
	}

	if(sql_stmt)
	{
		sqlite3_reset(sql_stmt);//clean it up so finalize can succeed.

		auto sqlite_result = 
		sqlite3_finalize(sql_stmt);

		if(sqlite_result != SQLITE_OK)
		{
			success = false;

			output_op_sql_error_message(db_connection, __LINE__);
		}
	}

	return std::pair<bool, int>(success, row_count);
}

//Callback function for sqlite3_exec.
//Defined to build a data structure, unit_type_query_values that represents a tabular result set.
//This result set can be traversed to calling process that issued the sql statement used to generate it.
static int 
translate_sql_result(void* user_defined_data, int column_count, char** column_values, char** column_names)
{
	unit_type_query_values* query_values = nullptr;

	if(user_defined_data)
	{
		query_values = static_cast<unit_type_query_values*>(user_defined_data);
	}

	unit_type_query_value row_of_data;//This is the actual row of data.

	//every column for the row should be accessible from here.
	//transfer each column to a corresponding column in the row_of_data variable.
	for(decltype(column_count) column_index = 0; column_index < column_count; column_index++)
	{
		const char* column_name_chars = column_names[column_index];
		const char* column_value_chars = column_values[column_index];

		if(query_values)
		{
			row_of_data[column_name_chars] = column_value_chars;
		}
		else if(_query_values_translator_output_enabled)
		{
			std::cout 
			<< "column name:" << column_name_chars 
			<< "| column value:" << column_value_chars 
			<< "\n";
		}
	}

	if(query_values)
	{
		query_values->push_back(row_of_data);
	}

	return 0;
}

//Goes through the query value lines. The value of the 
//	very first column key name that matches the input is returned.
static std::string 
get_first_string(const unit_type_query_value& row_of_data, const std::string& col_name)
{
	std::string col_value = "";

	for(auto& column : row_of_data)
	{
		if(column.first == col_name)
		{
			col_value = column.second;

			break;
		}
	}

	return col_value;
}

//SQL: Diagnostics

//Diagnostics support for SQLite3.
//----------------------------------------------------------
//The following functions are used for debugging, profiling use of SQLite3.
static void 
output_data_rows(const std::shared_ptr<unit_type_query_values> query_values)
{
	if(query_values)
	{
		for(const auto& row : *query_values)
		{
			for(const auto& column : row)
			{
				std::cout << "column name:" << column.first << "| column value:" << column.second << "\n";
			}
		}
	}

	return;
}

static void 
enable_op_sql_trace(sqlite3** db_connection)
{
	if(_sql_trace_enabled && !_sql_trace_active)
	{
		sqlite3_trace(*db_connection, trace_sql_op, nullptr);

		_sql_trace_active = true;
	}

	return;
}

static void 
trace_sql_op(void* in1, const char* in2)
{
	if(_sql_trace_active)
	{
		if(in1)
		{
			std::cout << " ";
		}

		std::cout << in2 << "\n";
	}

	return;
}

static void 
enable_op_sql_autolog()
{
	if(_sql_trace_enabled)
	{
		sqlite3_config(SQLITE_CONFIG_LOG, log_sql_op_event, nullptr);
	}

	return;
}

void 
log_sql_op_event(void *pArg, int iErrCode, const char *zMsg){
	if(_sql_trace_enabled && _sql_trace_active)
	{
		if(pArg)
		{
			std::cout << __FILE__ << "(" << __LINE__ << ") ";
		}

		std::cout 
		<< iErrCode << " " 
		<< zMsg;
	}

	return;
}

static void 
output_op_sql_error_message(sqlite3** db_connection, const int& line_number)
{
	std::cout 
	<< sqlite3_errmsg(*db_connection) << " " 
	<< line_number 
	<< "\n";

	return;
}

static void 
output_op_sql_error_message(char** error_message, const int& line_number)
{
	std::cout 
	<< *error_message << " " 
	<< line_number 
	<< "\n";

	sqlite3_free(*error_message);

	*error_message = nullptr;

	return;
}

//Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 . Software distributed under the License is distributed on an "AS IS" BASIS, NO WARRANTIES OR CONDITIONS OF ANY KIND, explicit or implicit. See the License for details on permissions and limitations.

