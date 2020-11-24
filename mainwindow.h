#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QMap>
#include <QMessageBox>
#include <QFileDialog>
#include <QIODevice>
#include <QCloseEvent>

#include "femcore.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QMap<int, Node> nodes;
    QMap<int, Rod> rods;
    QMap<int, Constraint> constraints;
    QMap<int, Load> loads;
    QMap<int, int> nodeInRods; // 在rod中node出现的次数

    QString curFile;
    bool solved;

private:
    void setLineEditValidator();
    bool okToContinue();
    void clear();

    enum { MagicNumber = 0x7F51C883 };

    bool loadFile(const QString &filename);
    bool saveFile(const QString &filename);
    void setCurrentFile(const QString &filename);
    QString strippedName(const QString &fullFileName);

    void setResultActionsEnabled(bool enabled);

private slots:
    void addNode();
    void deleteNode();
    void addRod();
    void deleteRod();
    void addConstraint();
    void deleteConstraint();
    void addLoad();
    void deleteLoad();
    void solve();

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void listNodes();
    void listRods();
    void listConstraints();
    void listLoads();
    void listNodesDisplacement();
    void listRodsInformation();
    void listConstraintForces();
    void showReset();
    void drawDeformedShape();
    void plotDefinedShape();
    void plotRodInformation();
    void plotConstraintForces();
};

#endif // MAINWINDOW_H
