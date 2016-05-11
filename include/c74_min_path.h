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
				case system::application:	pathid = max::path_getapppath(); break;
				case system::desktop: 		pathid = max::path_desktopfolder(); break;
				case system::preferences: 	max::preferences_path(nullptr, true, &pathid); break;
				case system::temp:		 	pathid = max::path_tempfolder(); break;
				default:					pathid = 0; break;
			}
		}
		
		~path() {
			;
		}
		
		operator std::string() const {
			char pathname[MAX_PATH_CHARS];
			
			// assuming folder, not file...
			max::path_topathname(pathid, nullptr, pathname);
			std::string s = pathname;
			return s;
		}
				
	private:
		short	pathid;
		
	};
	
}} // namespace c74::min
