#include "LayoutWatcher.h"
#include <sdbus-c++/sdbus-c++.h>
#include <QDebug>
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

LayoutWatcher::LayoutWatcher( QObject *parent ) : QObject( parent ) {
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
	delete fallbackX11_;
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
	emit onLayoutChanged( layoutsList_[id].shortName );
}

void LayoutWatcher::layoutListChanged() {
	updateLayouts();
	emit onLayoutListChanged( layoutsList_ );
}

void LayoutWatcher::createFallbackX11() {
	if ( fallbackX11_ ) return;
	qDebug() << "Fallback to X11";
	fallbackX11_ = new FallbackX11( consts::kFallbackUpdateTime, this );

	QObject::connect( fallbackX11_, &FallbackX11::onLayoutChanged, this, [this]( const std::string &layout ) {
		for ( size_t i = 0; i < layoutsList_.size(); ++i ) {
			if ( layoutsList_[i].shortName != layout ) continue;
			layoutId_ = i;
			break;
		}
		emit onLayoutChanged( layout );
	} );
	QObject::connect( fallbackX11_, &FallbackX11::onLayoutListChanged, this, [this]( const std::vector<LayoutNames> &layouts ) {
		layoutsList_ = layouts;
		emit onLayoutListChanged( layouts );
	} );
}
