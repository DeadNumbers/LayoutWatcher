#include "FallbackX11.h"
#include <iostream>
#if __has_include( <X11/XKBlib.h>)
#	include <X11/XKBlib.h>
#endif

FallbackX11::FallbackX11( std::chrono::milliseconds updateTime ) : updateTime_( updateTime ) {
#if __has_include( <X11/XKBlib.h>)
	thread_ = std::jthread( [this]( std::stop_token stoken ) { watcher( stoken ); } );
#endif
}

FallbackX11::~FallbackX11() {
#if __has_include( <X11/XKBlib.h>)
	thread_.request_stop();
	thread_.join();
	freeKeyboard();
#endif
}

std::vector<LayoutWatcher::LayoutNames> FallbackX11::getLayoutsList() const {
	return languagesToLayouts( getLanguages() );
}

void FallbackX11::updateDisplayAddr() {
	auto display_addr = getenv( "DISPLAY" );
	if ( !display_addr || !*display_addr ) return;
	if ( displayAddr_ != display_addr ) {
		displayAddr_ = display_addr;
		openKeyboard( displayAddr_ );
		updateLayouts();
	}
}

std::vector<FallbackX11::Language> FallbackX11::getLanguages() const {
	std::vector<Language> languages;
#if __has_include( <X11/XKBlib.h>)
	if ( !keyboard_ ) return languages;

	Atom current_group;
	for ( int i = 0; i < XkbNumKbdGroups; i++ ) {
		current_group = ( (XkbDescPtr)keyboard_ )->names->groups[i];
		if ( !current_group ) continue;

		auto group_name = XGetAtomName( (Display *)display_, current_group );
		if ( !group_name || !*group_name ) continue;

		Language lang;
		lang.name = group_name;
		lang.group = current_group;

		languages.push_back( lang );
		XFree( group_name );
	}
#endif

	return languages;
}

std::vector<LayoutWatcher::LayoutNames> FallbackX11::languagesToLayouts( const std::vector<Language> &languages ) const {
	std::vector<LayoutWatcher::LayoutNames> layoutsList;
	for ( auto &&lang : languages ) {
		LayoutWatcher::LayoutNames layout;
		layout.longName = lang.name;
		layout.displayName = layout.longName.substr( 0, 2 );
		layout.shortName = layout.displayName;
		std::transform( layout.shortName.begin(), layout.shortName.end(), layout.shortName.begin(), ::tolower );
		layoutsList.push_back( layout );
	}
	return layoutsList;
}

void FallbackX11::updateLayouts() {
#if __has_include( <X11/XKBlib.h>)
	if ( !keyboard_ ) return;

	auto languages = getLanguages();
	if ( !languages.empty() && languages_ != languages ) {
		languages_ = languages;
		auto layoutsList = languagesToLayouts( languages );
		onLayoutListChanged( layoutsList );
	}
#endif
}

bool FallbackX11::updateLayoutId( unsigned long group ) {
	if ( !group ) return false;
	if ( group == activeGroup_ ) return true;
#if __has_include( <X11/XKBlib.h>)
	activeGroup_ = group;
	for ( size_t i = 0; i < languages_.size(); ++i ) {
		if ( languages_[i].group != group ) continue;

		auto shortName = languages_[i].name.substr( 0, 2 );
		std::transform( shortName.begin(), shortName.end(), shortName.begin(), ::tolower );

		onLayoutChanged( shortName );
		return true;
	}
#endif
	return false;
}

unsigned long FallbackX11::getActiveGroup() {
	if ( !keyboard_ ) return 0;
#if __has_include( <X11/XKBlib.h>)
	XkbStateRec state;
	if ( XkbGetState( (Display *)display_, XkbUseCoreKbd, &state ) != Success ) {
		std::cerr << "Can't get active layout group" << std::endl;
		return 0;
	}
	return ( (XkbDescPtr)keyboard_ )->names->groups[state.group];
#endif
}

bool FallbackX11::freeKeyboard() {
#if __has_include( <X11/XKBlib.h>)
	if ( !keyboard_ ) return false;

	XkbFreeKeyboard( (XkbDescPtr)keyboard_, 0, True );
	keyboard_ = nullptr;
#endif
	return true;
}

void FallbackX11::watcher( std::stop_token &stoken ) {
	while ( !stoken.stop_requested() ) {
		updateDisplayAddr();
		auto group = getActiveGroup();
		if ( !updateLayoutId( group ) ) {
			openKeyboard( displayAddr_ );
			updateLayouts();
		}
		std::this_thread::sleep_for( updateTime_ );
	}
}

void FallbackX11::openKeyboard( std::string &display ) {
#if __has_include( <X11/XKBlib.h>)
	int result;
	display_ = XkbOpenDisplay( display.data(), NULL, NULL, NULL, NULL, &result );
	if ( !display_ ) {
		std::cerr << "Can't open display" << std::endl;
		return;
	}
	freeKeyboard();
	keyboard_ = XkbAllocKeyboard();
	if ( !keyboard_ ) {
		std::cerr << "Can't allocate keyboard" << std::endl;
		return;
	}
	if ( XkbGetNames( (Display *)display_, XkbGroupNamesMask, (XkbDescPtr)keyboard_ ) != Success ) {
		std::cerr << "Can't get keyboard names" << std::endl;
		freeKeyboard();
	}
#endif
}
