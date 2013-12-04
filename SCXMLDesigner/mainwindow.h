#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QWidget>
#include <QMainWindow>
#include <QStateMachine>
#include <QDomDocument>

#include "scxmlstate.h"
#include "workflow.h"
#include "workflowtab.h"
#include "utilities.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void CreateWidgets();
    void TempCreate();
    void CreateMenus();
    void CreateActions();
    void CreateToolbars();

private slots:
    void on_tabWidget_tabCloseRequested(int index);
    void insertState();
    void saveCurrentWorkflow();
    void loadWorkflow();
    WorkflowTab* createWorkflow();
    WorkflowTab* getActiveWorkflowTab();

private:
    QMenu *mMenuFile;
    QMenu *mMenuHelp;
    QMenu *mMenuEdit;
    QMenu *mMenuInsert;
    QMenu *mMenuTest;

    QAction *mActionNew;
    QAction *mActionOpen;
    QAction *mActionSave;
    QAction *mActionExit;
    QAction *mActionAbout;
    QAction *mActionState;
    QAction *mActionTransition;
    QAction *mActionShowChildStates;

    QToolBar *mFileToolBar;
    QToolBar *mInsertToolBar;

    QTabWidget *mTabWidget;

    QListWidget *mListOfStates;

    QWidget *mCentralWidget;
    QHBoxLayout *mHorizontalLayout;

    QToolBox *toolBox;
    QLCDNumber *lcdNumber;
    QWidget *page_1;
    QWidget *page_2;
};

#endif // MAINWINDOW_H
