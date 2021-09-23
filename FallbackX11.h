#pragma once

#include <chrono>

#if __has_include(<X11/XKBlib.h>)
#include <thread>
#include "LayoutWatcher.h"

class FallbackX11 : public QObject {
	Q_OBJECT
public:
	FallbackX11( std::chrono::milliseconds updateTime, QObject *parent = nullptr );
	virtual ~FallbackX11();

	QVector<LayoutWatcher::LayoutNames> getLayoutsList() const;

protected:
	struct Language {
		ulong group;
		QString name;

		bool operator==( const Language &rhs ) const { return group == rhs.group; }
	};

	std::chrono::milliseconds updateTime_;
	QString displayAddr_;
	ulong activeGroup_ = 0;
	QVector<Language> languages_;
	void *display_;
	void *keyboard_ = nullptr;
	std::jthread thread_;

	void updateDisplayAddr();
	QVector<Language> getLanguages() const;
	QVector<LayoutWatcher::LayoutNames> languagesToLayouts( const QVector<Language> &languages ) const;
	void updateLayouts();
	bool updateLayoutId( ulong group );

	ulong getActiveGroup();
	bool freeKeyboard();

	virtual void watcher( std::stop_token &stoken );

protected slots:
	void openKeyboard( const QString &display );

signals:
	void onLayoutListChanged( const QVector<LayoutWatcher::LayoutNames> &layouts );
	void onLayoutChanged( const QString &shortName );
};
#endif
