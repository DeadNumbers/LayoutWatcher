#include "LayoutWatcher.h"
#include <QtDBus>
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
}

LayoutWatcher::LayoutWatcher( QObject *parent ) : QObject( parent ) {
	qRegisterMetaType<QVector<LayoutNames>>( "QVector<LayoutNames>" );

	QDBusConnection bus = QDBusConnection::sessionBus();
	if ( !bus.isConnected() ) {
		qDebug() << "Cannot connect to the D-Bus session bus.";
		createFallbackX11();
		return;
	}

	qDBusRegisterMetaType<LayoutNames>();
	qDBusRegisterMetaType<QVector<LayoutNames>>();

	iface_ = new QDBusInterface( consts::kDBusService, consts::kDBusPath, consts::kDBusInterface, bus, this );

	if ( !bus.connect( iface_->service(), iface_->path(), iface_->interface(), consts::kDBusSigLayout, this, SLOT( layoutChanged(uint) ) ) ) {
		qDebug() << "layoutChanged not connected";
		createFallbackX11();
		return;
	}

	if ( !bus.connect( iface_->service(), iface_->path(), iface_->interface(), consts::kDBusSigLayoutList, this, SLOT( layoutListChanged() ) ) ) {
		qDebug() << "layoutListChanged not connected";
		createFallbackX11();
		return;
	}

	updateLayouts();

	QDBusReply<uint> layout = iface_->call( consts::kDBusMethodLayout );
	if ( layout.isValid() )
		layoutChanged( layout.value() );
	else
		createFallbackX11();
}

LayoutWatcher::~LayoutWatcher() {
	if ( fallbackX11_ ) {
		delete fallbackX11_;
		return;
	}

	QDBusConnection bus = QDBusConnection::sessionBus();
	if ( !bus.isConnected() ) return;

	bus.disconnect( iface_->service(), iface_->path(), iface_->interface(), consts::kDBusSigLayoutList, this, SLOT( layoutListChanged() ) );
	bus.disconnect( iface_->service(), iface_->path(), iface_->interface(), consts::kDBusSigLayout, this, SLOT( layoutChanged(uint) ) );

	delete iface_;
}

const QString &LayoutWatcher::getActiveLayout() const {
	return layoutsList_[layoutId_].shortName;
}

const QVector<LayoutWatcher::LayoutNames> &LayoutWatcher::getLayoutsList() const {
	return layoutsList_;
}

void LayoutWatcher::updateLayouts() {
	if ( fallbackX11_ ) {
		layoutsList_ = fallbackX11_->getLayoutsList();
		return;
	}
	QDBusReply<QVector<LayoutWatcher::LayoutNames>> layouts = iface_->call( consts::kDBusMethodLayoutList );
	if ( layouts.isValid() ) layoutsList_ = layouts.value();
}

void LayoutWatcher::layoutChanged( uint id ) {
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

	QObject::connect( fallbackX11_, &FallbackX11::onLayoutChanged, this, [this]( const QString &layout ) {
		for ( int i = 0; i < layoutsList_.count(); ++i ) {
			if ( layoutsList_[i].shortName != layout ) continue;
			layoutId_ = i;
			break;
		}
		emit onLayoutChanged( layout );
	} );
	QObject::connect( fallbackX11_, &FallbackX11::onLayoutListChanged, this, [this]( const QVector<LayoutNames> &layouts ) {
		layoutsList_ = layouts;
		emit onLayoutListChanged( layouts );
	} );
}

QDBusArgument &operator<<( QDBusArgument &argument, const LayoutWatcher::LayoutNames &layoutNames ) {
	argument.beginStructure();
	argument << layoutNames.shortName << layoutNames.displayName << layoutNames.longName;
	argument.endStructure();
	return argument;
}

const QDBusArgument &operator>>( const QDBusArgument &argument, LayoutWatcher::LayoutNames &layoutNames ) {
	argument.beginStructure();
	argument >> layoutNames.shortName >> layoutNames.displayName >> layoutNames.longName;
	argument.endStructure();
	return argument;
}
