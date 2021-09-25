#pragma once

#include "LayoutWatcher_global.h"
#include <memory>
#include <vector>
#include "eventpp/include/eventpp/callbacklist.h"

namespace sdbus {
	class IProxy;
}
class FallbackX11;

class LAYOUTWATCHER_EXPORT LayoutWatcher {
public:
	struct LayoutNames {
		std::string shortName;
		std::string displayName;
		std::string longName;

		bool operator==( const LayoutNames &rhs ) const {
			return shortName == rhs.shortName && displayName == rhs.displayName && longName == rhs.longName;
		}
	};

	[[maybe_unused]] LayoutWatcher();
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

	// Signals
	eventpp::CallbackList<void( const std::string & )> onLayoutChanged;
	eventpp::CallbackList<void( const std::vector<LayoutWatcher::LayoutNames> & )> onLayoutListChanged;

protected:
	std::unique_ptr<sdbus::IProxy> proxy_;
	unsigned int layoutId_;
	std::vector<LayoutNames> layoutsList_;

	void initialize( const char *service );

	// Fetch language layouts from DBus or X11
	void updateLayouts();

	// DBus signals for KDE session
	void layoutChanged( unsigned int id );
	void layoutListChanged();

private:
	std::unique_ptr<FallbackX11> fallbackX11_;

	void createFallbackX11();
};
