#include <QTime>
#include <QApplication>
#include <QAction>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QFileDialog>

#include "MainWindow.h"
#include "IndoorGML_writer.h"
#include "IndoorGML2LCC_Ops.h"

//#include <QOpenGLFunctions_2_1>
//#include "LCC_demo_plugin_interface.h"

extern std::map<str, Dart_handle> cellspace_dart;

class LCC_demo_IndoorGML2LCC_plugin :
        public QObject,
        public CGAL::LCC_demo_plugin_interface
{
    Q_OBJECT
    Q_INTERFACES(CGAL::LCC_demo_plugin_interface)
    Q_PLUGIN_METADATA(IID "LCC_Demo.PluginInterface/1.0")

public:
    void init (MainWindow *mymw, Scene *myscene)
    {
        scene = myscene;
        mw = mymw;

        QAction *importGML = new QAction("Import IndoorGML", mw);
        importGML->setProperty("subMenuName","IndoorGML");
        connect(importGML, SIGNAL(triggered()), this, SLOT(on_actionRead_IndoorGML_triggered()));
        _actions << importGML;

        QAction *addNRG = new QAction("Add NRG Layer (basic)", mw);
        addNRG->setProperty("subMenuName","IndoorGML");
        connect(addNRG, SIGNAL(triggered()), this, SLOT(on_actionAdd_NRG_triggered()));
        _actions << addNRG;

        QAction *exportGML = new QAction("Export IndoorGML", mw);
        exportGML->setProperty("subMenuName","IndoorGML");
        connect(exportGML, SIGNAL(triggered()), this, SLOT(on_actionWrite_IndoorGML_triggered()));
        _actions << exportGML;
    }

    QList<QAction*> actions()const {return _actions;}

    bool myIndoorGMLisEmpty()
    { return (myIndoorGML.getprimalSpaceFeatures()->getCellSpaceMember())->size() == 0; }

    void setIndoorFeatures( IndoorFeatures InFt )
    { myIndoorGML = InFt; }
    IndoorFeatures *getIndoorFeatures()
    { return &myIndoorGML; }


Q_SIGNALS:
    void sceneChangedByPlugin();/*{return;}*/


public Q_SLOTS:
  void on_actionAdd_NRG_triggered();
  void on_actionRead_IndoorGML_triggered();
  void on_actionWrite_IndoorGML_triggered();
  void load_IndoorGML(str);
  bool save_IndoorGML(str);

private:
  Scene* scene;
  MainWindow* mw;
  QList<QAction*> _actions;
  IndoorFeatures myIndoorGML;
};

void LCC_demo_IndoorGML2LCC_plugin::on_actionRead_IndoorGML_triggered()
{
    QString fileName = QFileDialog::getOpenFileName (mw,
                                                     tr ("Load"),
                                                     "./gml",
                                                     tr ("IndoorGML files (*.gml)"));

    if (!fileName.isEmpty ())
    {
        load_IndoorGML(fileName.toStdString());
        Q_EMIT(mw->sceneChanged());
    }

//    QOpenGLFunctions_2_1::glPointSize(7.0f);
//    QOpenGLFunctions_2_1::glBegin(GL_POINTS);
//    QOpenGLFunctions_2_1::glColor3f(.1f,.1f,.8f);
//    for( auto& it : cellspace_dart )
//    {
//        Bbox_3 bb = LCCtools::Get_Bbox_vol(*scene->lcc, it.second);
//        Point pt( (bb.xmax() + bb.xmin())/2.0,
//                  (bb.ymax() + bb.ymin())/2.0,
//                  (bb.zmax() + bb.zmin())/2.0);

//        QOpenGLFunctions_2_1::glVertex3f( pt.x(),pt.y(),pt.z());
//    }
//    QOpenGLFunctions_2_1::glEnd();
//    QOpenGLFunctions_2_1::glPointSize(4.f);

}

void LCC_demo_IndoorGML2LCC_plugin::load_IndoorGML(str myfile)
{

    IndoorFeatures InFt;
    readIndoorGML(*scene->lcc, InFt, myfile);
    std::cout << "The loaded LCC counts " << scene->lcc->number_of_vertex_attributes() << " vertices!" << std::endl;
    setIndoorFeatures( InFt );

    // Shifts only when necessary
    global_shift_pt = Point( 0.0, 0.0, 0.0 );
    LCCtools::Apply_global_shifting(*scene->lcc);

//    vec_dart to_clean;
//    LCCtools::Keep_corners_points_only(*scene->lcc, to_clean);

//    std::cout << "Number of 0-cell to be removed: " << to_clean.size() << std::endl;
//    LCCtools::Remove_selected_0_cells(*scene->lcc, to_clean);
//    std::cout << "\tDONE removing them all!" << std::endl;
}

void LCC_demo_IndoorGML2LCC_plugin::on_actionWrite_IndoorGML_triggered()
{
    if (!myIndoorGMLisEmpty())
    {
        QString fileName = QFileDialog::getSaveFileName (mw,
                                                         tr ("Export as an IndoorGML file"),
                                                         "myFile",
                                                         tr ("IndoorGML files (*.gml)"));
        if (!fileName.isEmpty())
        {
            if ( !save_IndoorGML(fileName.toStdString()) )
                std::cout << "\nCould not generate an IndoorGMLv1.0.3 file!" << std::endl;
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("No IndoorGML entity to export!");
        msgBox.exec();
    }
}

bool LCC_demo_IndoorGML2LCC_plugin::save_IndoorGML(str out)
{
    std::ofstream outfile (out.c_str());
    if  (outfile.is_open())
    {
        writeIndoorGML(*scene->lcc, *(getIndoorFeatures()), outfile );
        std::cout << "\nSuccessfully generated IndoorGMLv1.0.3 file: " << out << std::endl;
        return true;
    }
    else
    {
        std::cout << "Unable to open file" << std::endl;
        return false;
    }
}


void LCC_demo_IndoorGML2LCC_plugin::on_actionAdd_NRG_triggered()
{
    mw->on_actionSew3_same_facets_triggered();

#ifdef CGAL_PROFILE_LCC_DEMO
  CGAL::Timer timer;
  timer.start();
#endif

    mw->statusBar()->showMessage
            (QString ("Creating a Node Relation Graph (NRG) for connected 3-cells"), 1500);
    createNRGLayer( *scene->lcc, *(getIndoorFeatures()) );

  QApplication::restoreOverrideCursor ();

#ifdef CGAL_PROFILE_LCC_DEMO
  timer.stop();
  std::cout<<"Time to generate the NRG: "
           <<timer.time()<<" seconds."<<std::endl;
#endif

}


#include "IndoorGML2LCC.moc"
