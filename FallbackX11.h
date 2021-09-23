#pragma once

#include <chrono>
#include <thread>
#include "LayoutWatcher.h"

class FallbackX11 {
public:
	FallbackX11( std::chrono::milliseconds updateTime );
	virtual ~FallbackX11();

	std::vector<LayoutWatcher::LayoutNames> getLayoutsList() const;

	// Signals
	eventpp::CallbackList<void( std::string_view )> onLayoutChanged;
	eventpp::CallbackList<void( const std::vector<LayoutWatcher::LayoutNames> & )> onLayoutListChanged;

protected:
	struct Language {
		ulong group;
		std::string name;

		bool operator==( const Language &rhs ) const { return group == rhs.group; }
	};

	std::chrono::milliseconds updateTime_;
	std::string displayAddr_;
	unsigned long activeGroup_ = 0;
	std::vector<Language> languages_;
	void *display_;
	void *keyboard_ = nullptr;
	std::jthread thread_;

	void updateDisplayAddr();
	std::vector<Language> getLanguages() const;
	std::vector<LayoutWatcher::LayoutNames> languagesToLayouts( const std::vector<Language> &languages ) const;
	void updateLayouts();
	bool updateLayoutId( unsigned long group );

	unsigned long getActiveGroup();
	bool freeKeyboard();

	virtual void watcher( std::stop_token &stoken );

	void openKeyboard( std::string &display );
};
