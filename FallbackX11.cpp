#if __has_include(<X11/XKBlib.h>)
#include "FallbackX11.h"
#include <QDebug>
#include <X11/XKBlib.h>

FallbackX11::FallbackX11( std::chrono::milliseconds updateTime, QObject *parent ) : QObject( parent ), updateTime_( updateTime ) {
	thread_ = std::jthread( [this]( std::stop_token stoken ) { watcher( stoken ); } );
}

FallbackX11::~FallbackX11() {
	thread_.request_stop();
	thread_.join();
	freeKeyboard();
}

QVector<LayoutWatcher::LayoutNames> FallbackX11::getLayoutsList() const {
	return languagesToLayouts( getLanguages() );
}

void FallbackX11::updateDisplayAddr() {
	auto display_addr = getenv( "DISPLAY" );
	if ( !display_addr || !*display_addr ) return;
	if ( displayAddr_ != display_addr ) {
		displayAddr_ = display_addr;
		openKeyboard( displayAddr_ );
	}
}

QVector<FallbackX11::Language> FallbackX11::getLanguages() const {
	QVector<Language> languages;
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

		languages << lang;
		XFree( group_name );
	}

	return languages;
}

QVector<LayoutWatcher::LayoutNames> FallbackX11::languagesToLayouts( const QVector<Language> &languages ) const {
	QVector<LayoutWatcher::LayoutNames> layoutsList;
	for ( auto &&lang : languages ) {
		LayoutWatcher::LayoutNames layout;
		layout.longName = lang.name;
		layout.displayName = QString( layout.longName.data(), layout.longName.length() > 2 ? 2 : layout.longName.length() );
		layout.shortName = layout.displayName.toLower();
		layoutsList << layout;
	}
	return layoutsList;
}

void FallbackX11::updateLayouts() {
	if ( !keyboard_ ) return;

	auto languages = getLanguages();
	if ( !languages.empty() && languages_ != languages ) {
		languages_ = languages;
		auto layoutsList = languagesToLayouts( languages );
		emit onLayoutListChanged( layoutsList );
	}
}

bool FallbackX11::updateLayoutId( ulong group ) {
	if ( !group ) return false;
	if ( group == activeGroup_ ) return true;
	activeGroup_ = group;
	for ( int i = 0; i < languages_.count(); ++i ) {
		if ( languages_[i].group != group ) continue;

		auto &name = languages_[i].name;
		auto shortName = QString( name.data(), name.length() > 2 ? 2 : name.length() ).toLower();

		emit onLayoutChanged( shortName );
		return true;
	}
	return false;
}

ulong FallbackX11::getActiveGroup() {
	if ( !keyboard_ ) return 0;
	XkbStateRec state;
	if ( XkbGetState( (Display *)display_, XkbUseCoreKbd, &state ) != Success ) {
		qDebug() << "Can't get active layout group";
		return 0;
	}
	return ( (XkbDescPtr)keyboard_ )->names->groups[state.group];
}

bool FallbackX11::freeKeyboard() {
	if ( !keyboard_ ) return false;

	XkbFreeKeyboard( (XkbDescPtr)keyboard_, 0, True );
	keyboard_ = nullptr;
	return true;
}

void FallbackX11::watcher( std::stop_token &stoken ) {
	while ( !stoken.stop_requested() ) {
		updateDisplayAddr();
		auto group = getActiveGroup();
		if ( !updateLayoutId( group ) ) updateLayouts();
		std::this_thread::sleep_for( updateTime_ );
	}
}

void FallbackX11::openKeyboard( const QString &display ) {
	int result;
	display_ = XkbOpenDisplay( display.toLocal8Bit().data(), NULL, NULL, NULL, NULL, &result );
	if ( !display_ ) {
		qDebug() << "Can't open display";
		return;
	}
	freeKeyboard();
	keyboard_ = XkbAllocKeyboard();
	if ( !keyboard_ ) {
		qDebug() << "Can't allocate keyboard";
		return;
	}
	if ( XkbGetNames( (Display *)display_, XkbGroupNamesMask, (XkbDescPtr)keyboard_ ) != Success ) {
		qDebug() << "Can't get keyboard names";
		freeKeyboard();
	}
}
#endif
