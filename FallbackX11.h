#pragma once

#include <chrono>

#include <thread>
#include "LayoutWatcher.h"

class FallbackX11 : public QObject {
	Q_OBJECT
public:
	FallbackX11( std::chrono::milliseconds updateTime, QObject *parent = nullptr );
	virtual ~FallbackX11();

	std::vector<LayoutWatcher::LayoutNames> getLayoutsList() const;

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

protected slots:
	void openKeyboard( std::string &display );

signals:
	void onLayoutListChanged( const std::vector<LayoutWatcher::LayoutNames> &layouts );
	void onLayoutChanged( const std::string &shortName );
};
