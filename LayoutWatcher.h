#pragma once

#include "LayoutWatcher_global.h"
#include <QObject>
#include <memory>
#include <vector>

namespace sdbus {
	class IProxy;
}

class LAYOUTWATCHER_EXPORT LayoutWatcher : public QObject {
	Q_OBJECT

public:
	struct LayoutNames {
		std::string shortName;
		std::string displayName;
		std::string longName;
	};

	[[maybe_unused]] LayoutWatcher( QObject *parent = nullptr );
	virtual ~LayoutWatcher();

	/**
	 * @brief Get current language layout
	 * @return short name of active layout
	 */
	virtual const std::string &getActiveLayout() const;
	/**
	 * @brief Get list of language layouts
	 * @return list of layouts
	 */
	virtual const std::vector<LayoutWatcher::LayoutNames> &getLayoutsList() const;

protected:
	std::unique_ptr<sdbus::IProxy> proxy_;
	unsigned int layoutId_;
	std::vector<LayoutNames> layoutsList_;

	// Fetch language layouts from DBus or X11
	void updateLayouts();

protected slots: // DBus signals for KDE session
	void layoutChanged( unsigned int id );
	void layoutListChanged();

private:
	class FallbackX11 *fallbackX11_ = nullptr;

	void createFallbackX11();

signals:
	/**
	 * @brief Emited on change active layout
	 * @param shortName short name of new layout (e.g. en for English (US))
	 */
	void onLayoutChanged( const std::string &shortName );
	/**
	 * @brief Emited on change layout list
	 * @param layouts list with layouts
	 */
	void onLayoutListChanged( const std::vector<LayoutWatcher::LayoutNames> &layouts );
};
