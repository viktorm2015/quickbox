#ifndef CLASSESWIDGET_H
#define CLASSESWIDGET_H

#include "thispartwidget.h"
#include "coursedef.h"

#include <QFrame>

class QComboBox;

namespace Ui {
	class ClassesWidget;
}

namespace qf {
namespace core {
namespace model {
class SqlTableModel;
}
}
namespace qmlwidgets {
class ForeignKeyComboBox;
}
}

class ClassesWidget : public QFrame
{
	Q_OBJECT
private:
	typedef QFrame Super;
public:
	explicit ClassesWidget(QWidget *parent = 0);
	~ClassesWidget() Q_DECL_OVERRIDE;

	Q_INVOKABLE int selectedStageId();

	void settleDownInPartWidget(ThisPartWidget *part_widget);
private:
	Q_SLOT void import_ocad_v8();
	Q_SLOT void import_ocad_iofxml();
	Q_SLOT void reset();
	Q_SLOT void reload();
	Q_SLOT void reloadCourseCodes();
private:
	void importCourses(const QList<CourseDef> &course_defs);
private:
	Ui::ClassesWidget *ui;
	qf::core::model::SqlTableModel *m_classesModel;
	qf::core::model::SqlTableModel *m_courseCodesModel;
	QComboBox *m_cbxStage = nullptr;
};

#endif // CLASSESWIDGET_H
