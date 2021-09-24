#include "LayoutWatcher.h"
#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include "FallbackX11.h"

using namespace std::chrono_literals;
namespace consts {
	constexpr auto kDBusService = "org.kde.keyboard";
	constexpr auto kDBusPath = "/Layouts";
	constexpr auto kDBusInterface = "org.kde.KeyboardLayouts";
	// DBus signals
	constexpr auto kDBusSigLayout = "layoutChanged";
	constexpr auto kDBusSigLayoutList = "layoutListChanged";
	// DBus methods
	constexpr auto kDBusMethodLayout = "getLayout";
	constexpr auto kDBusMethodLayoutList = "getLayoutsList";

	constexpr auto kFallbackUpdateTime = 50ms;
} // namespace consts

LayoutWatcher::LayoutWatcher() {
	try {
		proxy_ = sdbus::createProxy( sdbus::createSessionBusConnection(), consts::kDBusService, consts::kDBusPath );
		proxy_->uponSignal( consts::kDBusSigLayout ).onInterface( consts::kDBusInterface ).call( [this]( unsigned int layoutId ) {
			layoutChanged( layoutId );
		} );
		proxy_->uponSignal( consts::kDBusSigLayoutList ).onInterface( consts::kDBusInterface ).call( [this] { layoutListChanged(); } );
		proxy_->finishRegistration();

		updateLayouts();

		unsigned int layoutId;
		proxy_->callMethod( consts::kDBusMethodLayout ).onInterface( consts::kDBusInterface ).storeResultsTo( layoutId );
		layoutChanged( layoutId );
	} catch ( const sdbus::Error &e ) { createFallbackX11(); }
}

LayoutWatcher::~LayoutWatcher() {
	fallbackX11_.reset();
	proxy_.reset();
}

const std::string &LayoutWatcher::getActiveLayout() const {
	return layoutsList_[layoutId_].shortName;
}

const std::vector<LayoutWatcher::LayoutNames> &LayoutWatcher::getLayoutsList() const {
	return layoutsList_;
}

void LayoutWatcher::updateLayouts() {
	layoutsList_.clear();
	if ( fallbackX11_ ) {
		layoutsList_ = fallbackX11_->getLayoutsList();
		return;
	}
	std::vector<sdbus::Struct<std::string, std::string, std::string>> layouts;
	proxy_->callMethod( consts::kDBusMethodLayoutList ).onInterface( consts::kDBusInterface ).storeResultsTo( layouts );
	for ( auto &&layout : layouts ) layoutsList_.emplace_back( layout.get<0>(), layout.get<1>(), layout.get<2>() );
}

void LayoutWatcher::layoutChanged( unsigned int id ) {
	layoutId_ = id;
	onLayoutChanged( layoutsList_[id].shortName );
}

void LayoutWatcher::layoutListChanged() {
	updateLayouts();
	onLayoutListChanged( layoutsList_ );
}

void LayoutWatcher::createFallbackX11() {
	if ( fallbackX11_ ) return;
	std::cerr << "Fallback to X11" << std::endl;
	fallbackX11_.reset( new FallbackX11( consts::kFallbackUpdateTime ) );

	fallbackX11_->onLayoutChanged.append( [this]( const std::string &layout ) {
		for ( size_t i = 0; i < layoutsList_.size(); ++i ) {
			if ( layoutsList_[i].shortName != layout ) continue;
			layoutId_ = i;
			break;
		}
		onLayoutChanged( layout );
	} );
	fallbackX11_->onLayoutListChanged.append( [this]( const std::vector<LayoutNames> &layouts ) {
		layoutsList_ = layouts;
		onLayoutListChanged( layouts );
	} );
}
