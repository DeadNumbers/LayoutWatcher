#pragma once

#include "LayoutWatcher_global.h"
#include <QObject>

class QDBusInterface;
class QDBusArgument;

class LAYOUTWATCHER_EXPORT LayoutWatcher : public QObject {
	Q_OBJECT

public:
	struct LayoutNames {
		QString shortName;
		QString displayName;
		QString longName;
	};

	[[maybe_unused]] LayoutWatcher( QObject *parent = nullptr );
	virtual ~LayoutWatcher();

	/**
	 * @brief Get current language layout
	 * @return short name of active layout
	 */
	virtual const QString &getActiveLayout() const;
	/**
	 * @brief Get list of language layouts
	 * @return list of layouts
	 */
	virtual const QVector<LayoutWatcher::LayoutNames> &getLayoutsList() const;

protected:
	QDBusInterface *iface_ = nullptr;
	uint layoutId_;
	QVector<LayoutNames> layoutsList_;

	// Fetch language layouts from DBus or X11
	void updateLayouts();

protected slots: // DBus signals for KDE session
	void layoutChanged( uint id );
	void layoutListChanged();

private:
	class FallbackX11 *fallbackX11_ = nullptr;

	void createFallbackX11();

signals:
	/**
	 * @brief Emited on change active layout
	 * @param shortName short name of new layout (e.g. en for English (US))
	 */
	void onLayoutChanged( const QString &shortName );
	/**
	 * @brief Emited on change layout list
	 * @param layouts list with layouts
	 */
	void onLayoutListChanged( const QVector<LayoutWatcher::LayoutNames> &layouts );
};

QDBusArgument &operator<<( QDBusArgument &argument, const LayoutWatcher::LayoutNames &layoutNames );
const QDBusArgument &operator>>( const QDBusArgument &argument, LayoutWatcher::LayoutNames &layoutNames );

Q_DECLARE_METATYPE( LayoutWatcher::LayoutNames )
