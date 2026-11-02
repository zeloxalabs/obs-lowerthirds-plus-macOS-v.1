/**
 * Lower Thirds Plus - Dock panel for OBS (View → Docks)
 * Author: Y Phic Hdok, MTD Technologies
 */

#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

#include <obs-frontend-api.h>
#include <obs-module.h>

class LowerThirdsDock : public QWidget {
	Q_OBJECT
public:
	explicit LowerThirdsDock(QWidget *parent = nullptr);
	~LowerThirdsDock() override;

	void refreshSourceList();

private Q_SLOTS:
	void onPlayTab(int index);
	void onHide();

private:
	QComboBox *m_sourceCombo = nullptr;
	QPushButton *m_play1 = nullptr;
	QPushButton *m_play2 = nullptr;
	QPushButton *m_play3 = nullptr;
	QPushButton *m_play4 = nullptr;
	QPushButton *m_play5 = nullptr;
	QPushButton *m_hide = nullptr;

	obs_source_t *getCurrentSource() const;
	static void frontendEvent(enum obs_frontend_event event, void *data);
};

void register_lowerthirds_dock();
