/**
 * Lower Thirds Plus - Dock panel for OBS (View → Docks)
 * Author: Y Phic Hdok, MTD Technologies
 */

#include "lowerthirds-dock.hpp"
#include <QGridLayout>
#include <QFrame>
#include <QString>

static LowerThirdsDock *s_dock = nullptr;
static void *s_eventCallback = nullptr;

LowerThirdsDock::LowerThirdsDock(QWidget *parent) : QWidget(parent)
{
	setWindowTitle("Lower Thirds Control");
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(8);

	QLabel *sourceLabel = new QLabel("Source:", this);
	mainLayout->addWidget(sourceLabel);
	m_sourceCombo = new QComboBox(this);
	m_sourceCombo->setMinimumWidth(200);
	mainLayout->addWidget(m_sourceCombo);

	QGroupBox *playGroup = new QGroupBox("Play tab", this);
	QHBoxLayout *playLayout = new QHBoxLayout(playGroup);
	m_play1 = new QPushButton("▶ 1", this);
	m_play2 = new QPushButton("▶ 2", this);
	m_play3 = new QPushButton("▶ 3", this);
	m_play4 = new QPushButton("▶ 4", this);
	m_play5 = new QPushButton("▶ 5", this);
	playLayout->addWidget(m_play1);
	playLayout->addWidget(m_play2);
	playLayout->addWidget(m_play3);
	playLayout->addWidget(m_play4);
	playLayout->addWidget(m_play5);
	mainLayout->addWidget(playGroup);

	m_hide = new QPushButton("Hide", this);
	mainLayout->addWidget(m_hide);

	connect(m_play1, &QPushButton::clicked, this, [this]() { onPlayTab(0); });
	connect(m_play2, &QPushButton::clicked, this, [this]() { onPlayTab(1); });
	connect(m_play3, &QPushButton::clicked, this, [this]() { onPlayTab(2); });
	connect(m_play4, &QPushButton::clicked, this, [this]() { onPlayTab(3); });
	connect(m_play5, &QPushButton::clicked, this, [this]() { onPlayTab(4); });
	connect(m_hide, &QPushButton::clicked, this, &LowerThirdsDock::onHide);

	refreshSourceList();
}

LowerThirdsDock::~LowerThirdsDock()
{
	if (s_eventCallback) {
		obs_frontend_remove_event_callback(frontendEvent, this);
		s_eventCallback = nullptr;
	}
	s_dock = nullptr;
}

void LowerThirdsDock::frontendEvent(enum obs_frontend_event event, void *data)
{
	Q_UNUSED(data);
	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED ||
	    event == OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED) {
		if (s_dock)
			s_dock->refreshSourceList();
	}
}

void LowerThirdsDock::refreshSourceList()
{
	m_sourceCombo->clear();
	m_sourceCombo->addItem("(No Lower Thirds Plus source in scene)", QString());

	obs_source_t *currentScene = obs_frontend_get_current_scene();
	if (!currentScene) return;

	obs_scene_t *scene = obs_scene_from_source(currentScene);
	if (!scene) {
		obs_source_release(currentScene);
		return;
	}

	struct enum_data { QComboBox *combo; } ed = { m_sourceCombo };
	auto enumItem = [](obs_scene_t *, obs_sceneitem_t *item, void *param) -> bool {
		enum_data *ed = (enum_data *)param;
		obs_source_t *src = obs_sceneitem_get_source(item);
		const char *id = obs_source_get_id(src);
		if (id && strcmp(id, "lowerthirds_plus_source") == 0) {
			const char *name = obs_source_get_name(src);
			ed->combo->addItem(name ? name : "Lower Thirds Plus", QString(name ? name : ""));
		}
		return true;
	};
	obs_scene_enum_items(scene, enumItem, &ed);
	obs_scene_release(scene);
	obs_source_release(currentScene);
}

static obs_source_t *find_source_in_scene_by_name(obs_scene_t *scene, const char *name)
{
	obs_source_t *found = nullptr;
	struct param { const char *name; obs_source_t *out; } p = { name, nullptr };
	auto enumItem = [](obs_scene_t *, obs_sceneitem_t *item, void *param) -> bool {
		struct param *p = (struct param *)param;
		obs_source_t *src = obs_sceneitem_get_source(item);
		if (strcmp(obs_source_get_name(src), p->name) == 0) {
			p->out = src;
			obs_source_addref(p->out);
			return false;
		}
		return true;
	};
	obs_scene_enum_items(scene, enumItem, &p);
	return p.out;
}

obs_source_t *LowerThirdsDock::getCurrentSource() const
{
	QString name = m_sourceCombo->currentData().toString();
	if (name.isEmpty()) return nullptr;
	obs_source_t *scene = obs_frontend_get_current_scene();
	if (!scene) return nullptr;
	obs_scene_t *s = obs_scene_from_source(scene);
	obs_source_t *found = find_source_in_scene_by_name(s, name.toUtf8().constData());
	obs_scene_release(s);
	obs_source_release(scene);
	return found;
}

void LowerThirdsDock::onPlayTab(int index)
{
	obs_source_t *src = getCurrentSource();
	if (!src) return;
	obs_data_t *settings = obs_source_get_settings(src);
	obs_data_set_int(settings, "dock_trigger", index);
	obs_source_update(src, settings);
	obs_data_release(settings);
	obs_source_release(src);
}

void LowerThirdsDock::onHide()
{
	obs_source_t *src = getCurrentSource();
	if (!src) return;
	obs_data_t *settings = obs_source_get_settings(src);
	obs_data_set_int(settings, "dock_trigger", 5);
	obs_source_update(src, settings);
	obs_data_release(settings);
	obs_source_release(src);
}

static void on_tools_menu_clicked(void *data)
{
	Q_UNUSED(data);
	// Tools item exists so you can find "Lower Thirds Control"; use View → Docks to open the panel.
	(void)0;
}

void register_lowerthirds_dock()
{
	obs_frontend_add_event_callback(frontendEvent, nullptr);
	s_eventCallback = (void *)1;

	s_dock = new LowerThirdsDock();
	bool ok = obs_frontend_add_dock_by_id("lowerthirds_plus_control", "Lower Thirds Control", s_dock);
	if (!ok && s_dock) {
		delete s_dock;
		s_dock = nullptr;
		blog(LOG_WARNING, "LowerThirdsPlus: Failed to add dock");
		return;
	}
	blog(LOG_INFO, "LowerThirdsPlus: Dock added (View → Docks → Lower Thirds Control)");

	obs_frontend_add_tools_menu_item("Lower Thirds Control", on_tools_menu_clicked, nullptr);
	blog(LOG_INFO, "LowerThirdsPlus: Tools menu item added (Tools → Lower Thirds Control)");
}
