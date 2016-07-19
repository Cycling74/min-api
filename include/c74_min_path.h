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

		path(system initial) {
			switch (initial) {
				case system::application:	m_path = max::path_getapppath(); break;
				case system::desktop: 		m_path = max::path_desktopfolder(); break;
				case system::preferences: 	max::preferences_path(nullptr, true, &m_path); break;
				case system::temp:		 	m_path = max::path_tempfolder(); break;
				default:					m_path = 0; break;
			}
		}
		
		path(short path_id) {
			m_path = path_id;
		}
		
		~path() {
			;
		}
		
		operator std::string() const {
			char pathname[MAX_PATH_CHARS];
			
			// assuming folder, not file...
			max::path_topathname(m_path, nullptr, pathname);
			std::string s = pathname;
			return s;
		}
				
	private:
		short	m_path;
		
	};
	
}} // namespace c74::min
