#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QMap>
#include <QMessageBox>
#include <QFileDialog>
#include <QIODevice>
#include <QCloseEvent>
#include <QStandardItemModel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QKeyEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

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
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    int elementType;
    QMap<int, Material> materials;
    QMap<int, Section> sections;
    QMap<int, Offset> offsets;
    QMap<int, Node> nodes;
    QMap<int, Element> elements;
    QMap<int, Constraint> constraints;
    QMap<int, Load> loads;
    QMap<int, int> nodeInElements; // 在element中node出现的次数

    QString curFile;
    bool solved;

    QTreeWidgetItem *PreprocessorItem;
    QTreeWidgetItem *ElementTypeItem;
    QTreeWidgetItem *MaterialsItem;
    QTreeWidgetItem *AddMaterialItem;
    QTreeWidgetItem *SectionsItem;
    QTreeWidgetItem *AddSectionItem;
    QTreeWidgetItem *OffsetsItem;
    QTreeWidgetItem *AddOffsetItem;
    QTreeWidgetItem *NodesItem;
    QTreeWidgetItem *AddNodeItem;
    QTreeWidgetItem *ElementsItem;
    QTreeWidgetItem *AddElementItem;
    QTreeWidgetItem *ConstraintsItem;
    QTreeWidgetItem *AddConstraintItem;
    QTreeWidgetItem *LoadsItem;
    QTreeWidgetItem *AddLoadItem;
    QTreeWidgetItem *SolveItem;
    QTreeWidgetItem *ExportItem;

private:
    void setSolved(bool solved);
    void setupUI();
    bool okToContinue();
    void clear();

    enum { MagicNumber = 0x7F51C883 };

    bool loadFile(const QString &filename);
    bool saveFile(const QString &filename);
    void setCurrentFile(const QString &filename);
    QString strippedName(const QString &fullFileName);

    bool selectElementType();
    bool inputMaterial(Material &material, bool flag);
    bool inputSection(Section &section, bool flag);
    bool inputOffset(Offset &offset, bool flag);
    bool inputNode(Node &node, bool flag);
    bool inputElement(Element &element, bool flag);
    bool inputConstraint(Constraint &constraint, bool flag);
    bool inputLoad(Load &load, bool flag);
    void solve();

    void setResultActionsEnabled(bool enabled);
    void exportReport();

private slots:
    void treeWidgetItemDoubleClick(QTreeWidgetItem*, int);
    void treeWidgetItemPopMenu(const QPoint&);
    void deleteItem();

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void userguide();
    void listMaterials();
    void listSections();
    void listOffsets();
    void listNodes();
    void listElements();
    void listConstraints();
    void listLoads();
    void listNodesDisplacement();
    void listElementsInformation();
    void listConstraintForces();
    void showReset();
    void drawDeformedShape();
    void plotDefinedShape();
    void plotElementInformation();
    void plotConstraintForces();
};

#endif // MAINWINDOW_H
