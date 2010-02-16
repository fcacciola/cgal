#include "config.h"
#ifdef CGAL_POLYHEDRON_DEMO_USE_SURFACE_MESHER
#include "Polyhedron_demo_plugin_helper.h"
#include "Polyhedron_demo_plugin_interface.h"
#include "ui_Smoother_dialog.h"
#include "ui_LocalOptim_dialog.h"

#include "Scene_c3t3_item.h"
#include "C3t3_type.h"

#include <QObject>
#include <QAction>
#include <QMainWindow>
#include <QApplication>
#include <QtPlugin>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <fstream>

#include <CGAL/optimize_mesh_3.h> // to get default values


// declare the CGAL function
Scene_c3t3_item* cgal_code_odt_mesh_3(Scene_c3t3_item& c3t3_item,
                                      const double time_limit,
                                      const double convergence_ratio,
                                      const double freeze_ratio,
                                      const int max_iteration_number,
                                      const bool create_new_item);

Scene_c3t3_item* cgal_code_lloyd_mesh_3(Scene_c3t3_item& c3t3_item,
                                        const double time_limit,
                                        const double convergence_ratio,
                                        const double freeze_ratio,
                                        const int max_iteration_number,
                                        const bool create_new_item);

Scene_c3t3_item* cgal_code_perturb_mesh_3(Scene_c3t3_item& c3t3_item,
                                          const double time_limit,
                                          const double sliver_bound,
                                          const bool create_new_item);

Scene_c3t3_item* cgal_code_exude_mesh_3(Scene_c3t3_item& c3t3_item,
                                        const double time_limit,
                                        const double sliver_bound,
                                        const bool create_new_item);


// Mesh_3_demo_optimization_plugin class
class Mesh_3_demo_optimization_plugin : 
  public QObject,
  protected Polyhedron_demo_plugin_helper
{
  Q_OBJECT
  Q_INTERFACES(Polyhedron_demo_plugin_interface);
public:
  void init(QMainWindow* mainWindow, Scene_interface* scene_interface);
  inline QList<QAction*> actions() const;
  
public slots:
  void odt();
  void lloyd();
  void perturb();
  void exude();
  
private:
  void treat_result(Scene_c3t3_item& source_item, Scene_c3t3_item& result_item,
                    const QString& name, bool new_item_created) const;

private:
  QAction* actionOdt;
  QAction* actionLloyd;
  QAction* actionPerturb;
  QAction* actionExude;
}; // end class Mesh_3_demo_optimization_plugin


void 
Mesh_3_demo_optimization_plugin::
init(QMainWindow* mainWindow, Scene_interface* scene_interface)
{
  this->scene = scene_interface;
  this->mw = mainWindow;
  
  // Create menu items
  actionOdt = this->getActionFromMainWindow(mw, "actionOdt");
  if( NULL != actionOdt )
  {
    connect(actionOdt, SIGNAL(triggered()), this, SLOT(odt()));
  }
  
  actionLloyd = this->getActionFromMainWindow(mw, "actionLloyd");
  if( NULL != actionLloyd )
  {
    connect(actionLloyd, SIGNAL(triggered()), this, SLOT(lloyd()));
  }
  
  actionPerturb = this->getActionFromMainWindow(mw, "actionPerturb");
  if( NULL != actionPerturb )
  {
    connect(actionPerturb, SIGNAL(triggered()), this, SLOT(perturb()));
  }
  
  actionExude = this->getActionFromMainWindow(mw, "actionExude");
  if( NULL != actionExude )
  {
    connect(actionExude, SIGNAL(triggered()), this, SLOT(exude()));
  }
}


inline
QList<QAction*> 
Mesh_3_demo_optimization_plugin::actions() const
{
  return QList<QAction*>() << actionOdt << actionLloyd 
                           << actionPerturb << actionExude;
}


void
Mesh_3_demo_optimization_plugin::odt()
{
  // -----------------------------------
  // Get source item
  // -----------------------------------
  const Scene_interface::Item_id index = scene->mainSelectionIndex();
  Scene_c3t3_item* item = qobject_cast<Scene_c3t3_item*>(scene->item(index));
  
  if ( NULL == item )
  {
    return;
  }
  
  if ( NULL == item->data_item() )
  {
    QMessageBox::critical(mw,tr(""),
                          tr("Can't optimize: data object has been destroyed !"));
    return;
  }

  // -----------------------------------
  // Dialog box
  // -----------------------------------
  QDialog dialog(mw);
  Ui::Smoother_dialog ui;
  ui.setupUi(&dialog);
  dialog.setWindowTitle(tr("Odt-smoothing parameters"));
  
  connect(ui.buttonBox, SIGNAL(accepted()),
          &dialog, SLOT(accept()));
  connect(ui.buttonBox, SIGNAL(rejected()),
          &dialog, SLOT(reject()));
  
  ui.objectName->setText(item->name());

  namespace cgpd = CGAL::parameters::default_values;
  ui.convergenceRatio->setValue(cgpd::odt_convergence_ratio);
  ui.freezeRatio->setValue(cgpd::odt_freeze_ratio);
  
  int i = dialog.exec();
  if(i == QDialog::Rejected)
    return;
    
  // 0 means parameter is not considered
  const double max_time = ui.noTimeLimit->isChecked() ? 0 : ui.maxTime->value();
  const int max_iteration_nb = ui.maxIterationNb->value();
  const double convergence = ui.convergenceRatio->value();
  const double freeze = ui.freezeRatio->value();
  const bool create_new_item = ui.createNewItem->isChecked();
  
  // -----------------------------------
  // Launch optimization
  // -----------------------------------
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Scene_c3t3_item* result_item = cgal_code_odt_mesh_3(*item,
                                                      max_time,
                                                      convergence,
                                                      freeze,
                                                      max_iteration_nb,
                                                      create_new_item);
  
  if ( NULL == result_item )
  {
    QApplication::restoreOverrideCursor();
    return;
  }
    
  // -----------------------------------
  // Treat result
  // -----------------------------------
  QString name("odt");
  treat_result(*item, *result_item, name, create_new_item);
  
  QApplication::restoreOverrideCursor();
}


void
Mesh_3_demo_optimization_plugin::lloyd()
{
  // -----------------------------------
  // Get source item
  // -----------------------------------
  const Scene_interface::Item_id index = scene->mainSelectionIndex();
  Scene_c3t3_item* item = qobject_cast<Scene_c3t3_item*>(scene->item(index));
  
  if ( NULL == item )
  {
    return;
  }
  
  if ( NULL == item->data_item() )
  {
    QMessageBox::critical(mw,tr(""),
                          tr("Can't optimize: data object has been destroyed !"));
    return;
  }
  
  // -----------------------------------
  // Dialog box
  // -----------------------------------
  QDialog dialog(mw);
  Ui::Smoother_dialog ui;
  ui.setupUi(&dialog);
  dialog.setWindowTitle(tr("Lloyd-smoothing parameters"));
  
  connect(ui.buttonBox, SIGNAL(accepted()),
          &dialog, SLOT(accept()));
  connect(ui.buttonBox, SIGNAL(rejected()),
          &dialog, SLOT(reject()));
  
  ui.objectName->setText(item->name());
  
  namespace cgpd = CGAL::parameters::default_values;
  ui.convergenceRatio->setValue(cgpd::lloyd_convergence_ratio);
  ui.freezeRatio->setValue(cgpd::lloyd_freeze_ratio);
  
  int i = dialog.exec();
  if(i == QDialog::Rejected)
    return;
  
  // 0 means parameter is not considered
  const double max_time = ui.noTimeLimit->isChecked() ? 0 : ui.maxTime->value();
  const int max_iteration_nb = ui.maxIterationNb->value();
  const double convergence = ui.convergenceRatio->value();
  const double freeze = ui.freezeRatio->value();
  const bool create_new_item = ui.createNewItem->isChecked();

  // -----------------------------------
  // Launch optimization
  // -----------------------------------
  QApplication::setOverrideCursor(Qt::WaitCursor);
  
  Scene_c3t3_item* result_item = cgal_code_lloyd_mesh_3(*item,
                                                        max_time,
                                                        convergence,
                                                        freeze,
                                                        max_iteration_nb,
                                                        create_new_item);
  
  if ( NULL == result_item )
  {
    QApplication::restoreOverrideCursor();
    return;
  }
  
  // -----------------------------------
  // Treat result
  // -----------------------------------
  QString name("lloyd");
  treat_result(*item, *result_item, name, create_new_item);
  
  QApplication::restoreOverrideCursor();
}


void
Mesh_3_demo_optimization_plugin::perturb()
{
  // -----------------------------------
  // Get source item
  // -----------------------------------
  const Scene_interface::Item_id index = scene->mainSelectionIndex();
  Scene_c3t3_item* item = qobject_cast<Scene_c3t3_item*>(scene->item(index));
  
  if ( NULL == item )
  {
    return;
  }
  
  if ( NULL == item->data_item() )
  {
    QMessageBox::critical(mw,tr(""),
                          tr("Can't perturb: data object has been destroyed !"));
    return;
  }
  
  // -----------------------------------
  // Dialog box
  // -----------------------------------
  QDialog dialog(mw);
  Ui::LocalOptim_dialog ui;
  ui.setupUi(&dialog);
  dialog.setWindowTitle(tr("Sliver perturbation parameters"));
  
  connect(ui.buttonBox, SIGNAL(accepted()),
          &dialog, SLOT(accept()));
  connect(ui.buttonBox, SIGNAL(rejected()),
          &dialog, SLOT(reject()));
  
  ui.objectName->setText(item->name());
  
  int i = dialog.exec();
  if(i == QDialog::Rejected)
    return;
  
  // 0 means parameter is not considered
  const double max_time = ui.noTimeLimit->isChecked() ? 0 : ui.maxTime->value();
  const double sliver_bound = ui.noBound->isChecked() ? 0 : ui.sliverBound->value();
  const bool create_new_item = ui.createNewItem->isChecked();
  
  // -----------------------------------
  // Launch optimization
  // -----------------------------------
  QApplication::setOverrideCursor(Qt::WaitCursor);
  
  Scene_c3t3_item* result_item = cgal_code_perturb_mesh_3(*item,
                                                          max_time,
                                                          sliver_bound,
                                                          create_new_item);
  

  if ( NULL == result_item )
  {
    QApplication::restoreOverrideCursor();
    return;
  }
  
  // -----------------------------------
  // Treat result
  // -----------------------------------
  QString name("perturb");
  treat_result(*item, *result_item, name, create_new_item);

  QApplication::restoreOverrideCursor();
}


void
Mesh_3_demo_optimization_plugin::exude()
{
  // -----------------------------------
  // Get source item
  // -----------------------------------
  const Scene_interface::Item_id index = scene->mainSelectionIndex();
  Scene_c3t3_item* item = qobject_cast<Scene_c3t3_item*>(scene->item(index));
  
  if ( NULL == item )
  {
    return;
  }
  
  // -----------------------------------
  // Dialog box
  // -----------------------------------
  QDialog dialog(mw);
  Ui::LocalOptim_dialog ui;
  ui.setupUi(&dialog);
  dialog.setWindowTitle(tr("Sliver exudation parameters"));
  
  connect(ui.buttonBox, SIGNAL(accepted()),
          &dialog, SLOT(accept()));
  connect(ui.buttonBox, SIGNAL(rejected()),
          &dialog, SLOT(reject()));
  
  ui.objectName->setText(item->name());
  ui.sliverBound->setValue(25.);
  
  int i = dialog.exec();
  if(i == QDialog::Rejected)
    return;
  
  // 0 means parameter is not considered
  const double max_time = ui.noTimeLimit->isChecked() ? 0 : ui.maxTime->value();
  const double sliver_bound = ui.noBound->isChecked() ? 0 : ui.sliverBound->value();
  const bool create_new_item = ui.createNewItem->isChecked();
  
  // -----------------------------------
  // Launch optimization
  // -----------------------------------
  QApplication::setOverrideCursor(Qt::WaitCursor);
  
  Scene_c3t3_item* result_item = cgal_code_exude_mesh_3(*item,
                                                        max_time,
                                                        sliver_bound,
                                                        create_new_item);

  if ( NULL == result_item )
  {
    QApplication::restoreOverrideCursor();
    return;
  }
  
  // -----------------------------------
  // Treat result
  // -----------------------------------
  QString name("exude");
  treat_result(*item, *result_item, name, create_new_item);
  
  QApplication::restoreOverrideCursor();
}



void
Mesh_3_demo_optimization_plugin::
treat_result(Scene_c3t3_item& source_item,
             Scene_c3t3_item& result_item,
             const QString& name,
             bool new_item_created) const
{
  result_item.setName(tr("%1 [%2]").arg(source_item.name())
                                   .arg(name));
  
  if ( new_item_created )
  {
    result_item.setColor(Qt::magenta);
    result_item.setRenderingMode(source_item.renderingMode());
    result_item.set_data_item(source_item.data_item());
    
    source_item.setVisible(false);
    
    const Scene_interface::Item_id index = scene->mainSelectionIndex();
    scene->itemChanged(index);
    
    Scene_interface::Item_id new_item_id = scene->addItem(&result_item);
    scene->setSelectedItem(new_item_id);
  }
  else
  {
    const Scene_interface::Item_id index = scene->mainSelectionIndex();
    scene->itemChanged(index);
  }
}


Q_EXPORT_PLUGIN2(Mesh_3_demo_optimization_plugin, Mesh_3_demo_optimization_plugin);

#include "Mesh_3_demo_optimization_plugin.moc"

#endif // CGAL_POLYHEDRON_DEMO_USE_SURFACE_MESHER