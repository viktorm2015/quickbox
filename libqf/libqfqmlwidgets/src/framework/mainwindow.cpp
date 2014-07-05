#include "mainwindow.h"
#include "application.h"
#include "pluginloader.h"
#include "dockwidget.h"
#include "partwidget.h"
#include "stackedcentralwidget.h"
#include "../menubar.h"
#include "../statusbar.h"

#include <qf/core/log.h>
#include <qf/core/assert.h>
#include <qf/core/utils.h>
//#include <qf/core/settings.h>
//#include <qf/core/utils/crypt.h>

#include <QQmlEngine>
#include <QQmlContext>
#include <QSettings>
#include <QCloseEvent>

using namespace qf::qmlwidgets::framework;

MainWindow* MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags), IPersistentSettings(this)
{
	Q_ASSERT(self == nullptr);
	self = this;

	m_pluginLoader = nullptr;

	QQmlEngine *qe = Application::instance()->qmlEngine();
	qe->rootContext()->setContextProperty("FrameWork", this);
}

MainWindow::~MainWindow()
{
	savePersistentSettings();
}

void MainWindow::loadPlugins()
{
	QF_SAFE_DELETE(m_pluginLoader);
	m_pluginLoader = new PluginLoader(this);
	connect(m_pluginLoader, &PluginLoader::loadingFinished, this, &MainWindow::pluginsLoaded, Qt::QueuedConnection);
	connect(m_pluginLoader, &PluginLoader::loadingFinished, this, &MainWindow::whenPluginsLoaded, Qt::QueuedConnection);
	m_pluginLoader->loadPlugins();
}

void MainWindow::loadPersistentSettings()
{
	QString path = persistentSettingsPath();
	qfLogFuncFrame() << path;
	if(!path.isEmpty()) {
		QSettings settings;
		settings.beginGroup(path);
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("state").toByteArray());
	}
}

MainWindow *MainWindow::frameWork()
{
	Q_ASSERT(self != nullptr);
	return self;
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
	emit aboutToClose();
	ev->accept();
}

void MainWindow::savePersistentSettings()
{
	QString path = persistentSettingsPath();
	qfLogFuncFrame() << path;
	if(!path.isEmpty()) {
		QSettings settings;
		settings.beginGroup(path);
		settings.setValue("state", saveState());
		settings.setValue("geometry", saveGeometry());
	}
}

void MainWindow::whenPluginsLoaded()
{
	centralWidget()->setPartActive(0, true);
}

void MainWindow::setPersistentSettingDomains(const QString &organization_domain, const QString &organization_name, const QString &application_name)
{
	QCoreApplication::setOrganizationDomain(organization_domain);
	QCoreApplication::setOrganizationName(organization_name);
	if(!application_name.isEmpty()) {
		QCoreApplication::setApplicationName(application_name);
	}
	//QSettings::setDefaultFormat(QSettings::IniFormat);
}

qf::qmlwidgets::MenuBar *MainWindow::menuBar()
{
	QMenuBar *mb = Super::menuBar();
	MenuBar *menu_bar = qobject_cast<MenuBar*>(mb);
	if(!menu_bar) {
		QF_SAFE_DELETE(mb);
		menu_bar = new MenuBar(this);
		Super::setMenuBar(menu_bar);
	}
	return menu_bar;
}

qf::qmlwidgets::StatusBar *MainWindow::statusBar()
{
	QStatusBar *sb = Super::statusBar();
	StatusBar *status_bar = qobject_cast<StatusBar*>(sb);
	if(!status_bar) {
		QF_SAFE_DELETE(sb);
		status_bar = new StatusBar(this);
		Super::setStatusBar(status_bar);
	}
	return status_bar;
}

void MainWindow::setStatusBar(qf::qmlwidgets::StatusBar *sbar)
{
	//QStatusBar *sb = Super::statusBar();
	qfLogFuncFrame() << sbar << "previous:" << Super::statusBar();
	//QF_SAFE_DELETE(sb);
	sbar->setParent(0);
	//sbar->setParent(this);
	//sbar->setVisible(true);
	Super::setStatusBar(sbar); /// deletes old status bar
	//sbar->showMessage("ahoj babi");
	qfDebug() << Super::statusBar();
}

CentralWidget *MainWindow::centralWidget()
{
	QWidget *cw = Super::centralWidget();
	CentralWidget *central_widget = qobject_cast<CentralWidget*>(cw);
	if(!central_widget) {
		QF_SAFE_DELETE(cw);
		central_widget = new StackedCentralWidget(this);
		Super::setCentralWidget(central_widget);
	}
	return central_widget;
}

void MainWindow::setCentralWidget(CentralWidget *widget)
{
	qfLogFuncFrame() << widget;
	widget->setParent(0);
	Super::setCentralWidget(widget);
}

void MainWindow::addDockWidget(Qt::DockWidgetArea area, DockWidget *dockwidget)
{
	dockwidget->setParent(0);
	Super::addDockWidget(area, dockwidget);
}

void MainWindow::addPartWidget(PartWidget *widget, const QString &feature_id)
{
	if(!feature_id.isEmpty()) {
		if(widget->featureId().isEmpty()) {
			qfDebug() << "setting" << widget << "featureId to:" << feature_id;
			widget->setFeatureId(feature_id);
		}
		else if(widget->featureId() != feature_id)
			qfWarning() << "different featureIds set:" << feature_id << "vs." << widget->featureId() << ", the second one will be used.";
	}
	if(widget->featureId().isEmpty())
		qfWarning() << widget << "adding part widget without featureId set can harm some default functionality.";
	centralWidget()->addPartWidget(widget);
}

Plugin *MainWindow::plugin(const QString &feature_id)
{
	Plugin *ret = nullptr;
	if(m_pluginLoader) {
		ret = m_pluginLoader->loadedPlugins().value(feature_id);
	}
	if(!ret) {
		qfWarning() << "Plugin for feature id:" << feature_id << "is not installed!";
		qfWarning() << "Available feature ids:" << QStringList(m_pluginLoader->loadedPlugins().keys()).join(",");
	}
	return ret;
}
#if 0
class TestObject : public QObject
{
	Q_OBJECT
public:
	TestObject(QObject *parent = 0) : QObject(parent)
	{
		static int cnt = 0;
		setObjectName(QString("OBJ%1").arg(++cnt));
		//QQmlEngine::setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
		qfInfo() << "CREATE" << this << "ownersip:" << QQmlEngine::objectOwnership(this);
	}
	~TestObject()
	{
		qfInfo() << "DESTROY" << this << "ownersip:" << QQmlEngine::objectOwnership(this);
	}
	Q_INVOKABLE QObject* parent() {return QObject::parent();}
};

QObject *MainWindow::obj_testing()
{
	QObject *ret = new TestObject();
	return ret;
}

//#include "mainwindow.moc"
#endif
