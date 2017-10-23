/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {	

	
	class path {
	public:
		
		enum class system {
			undefined = 0,
			application,
			desktop,
			preferences,
			temp
		};


		enum class filetype {
			any = 0,
			folder,
			external,
			patcher,
			audio
		};


		using filedate = max::t_ptr_uint;


		// uninitialized path
		path() {}


		// path initialized to a system directory
		path(system initial)
		: m_directory {true}
		{
			switch (initial) {
				case system::application:	m_path = max::path_getapppath(); break;
				case system::desktop: 		m_path = max::path_desktopfolder(); break;
				case system::preferences: 	max::preferences_path(nullptr, true, &m_path); break;
				case system::temp:		 	m_path = max::path_tempfolder(); break;
				default:					m_path = 0; break;
			}
		}


		// path initialized to a user-supplied path id (discouraged, but might be provided by legacy Max API)
		path(short path_id)
		: m_path		{ path_id }
		, m_directory	{true}
		{}


		// path initialized by name
		path(const std::string& name, filetype type = filetype::any, bool create = false) {
			strncpy(m_filename, name.c_str(), MAX_PATH_CHARS);

			max::t_fourcc	types[max::TYPELIST_SIZE];
			short			type_count = 0;

			if (type == filetype::external)
				max::typelist_make(types, max::TYPELIST_EXTERNS, &type_count);
			else if (type == filetype::folder)
				m_directory = true;

			auto err = max::locatefile_extended(m_filename, &m_path, &m_type, types, type_count);
			if (err) {
				if (create) {
					if (type == filetype::folder) {
						char	fullpath[MAX_PATH_CHARS];
						char	filename[MAX_FILENAME_CHARS];
						max::path_nameconform(name.c_str(), fullpath, max::PATH_STYLE_MAX, max::PATH_TYPE_ABSOLUTE);

						char*	foldername = strrchr(fullpath, '/');
						short	parent_folder_path = 0;

						if (foldername) {
							*foldername = 0;
							foldername++;

							err = max::path_frompathname(fullpath, &parent_folder_path, filename);
							if (!err)
								err = max::path_createfolder(parent_folder_path, foldername, &m_path);
							if (err)
								error("error trying to create folder");
						}
						else
							error("no folder name provided");

						m_filename[0] = 0;
					}
					else {
						error("can only create folders, not files");
					}
				}
				else {
					error("file not found");
				}
			}
		}


		operator string() const {
			char pathname[MAX_PATH_CHARS];

			max::path_toabsolutesystempath(m_path, m_filename, pathname);
			std::string s = pathname;
			return s;
		}


		// is this a valid path?
		operator bool() const {
			return m_path != 0;
		}


		filedate date_modified() const {
			int			err;
			filedate	date;

			if (m_directory)
				err = max::path_getmoddate(m_path, &date);
			else
				err = max::path_getfilemoddate(m_filename, m_path, &date);

			if (err || date == 0) {
				error("failed to get date modified");
			}
			return date;
		}

				
	private:
		short			m_path = 0;
		char			m_filename[MAX_PATH_CHARS] = {};
		max::t_fourcc	m_type = 0;
		bool			m_directory = false;
	};
	
}} // namespace c74::min
