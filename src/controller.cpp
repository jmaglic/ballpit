
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include "misc.h"
#include "exception.h"
#include "special_chars.h"
#include <chrono>
#include <utility>
#include <map>

///////////////////////
// STATIC ATTRIBUTES //
///////////////////////

Ctrl* Ctrl::instance = NULL;
MainFrame* Ctrl::gui = NULL;

////////////////
// TOGGLE GUI //
////////////////

void Ctrl::disableGUI(){
  _to_gui = false;
}

void Ctrl::enableGUI(){
  _to_gui = true;
}

bool Ctrl::isGUIEnabled(){
  return _to_gui;
}

/////////////
// METHODS //
/////////////

void Ctrl::registerView(MainFrame* inp_gui){
  gui = inp_gui;
}

Ctrl* Ctrl::getInstance(){
  if(instance == NULL){
    instance = new Ctrl();
  }
  return instance;
}

bool Ctrl::loadRadiusFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  std::string radius_filepath = gui->getRadiusFilepath();
  // even if there is no valid radii file, the program can be used by manually setting radii in the GUI after loading a structure
  if(!current_calculation->readRadiusFileSetMaps(radius_filepath)){
    displayErrorMessage(101);
  }
  // refresh atom list using new radius map
  gui->displayAtomList(current_calculation->generateAtomList());
  return true;
}

bool Ctrl::loadAtomFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  bool successful_import;
  try{successful_import = current_calculation->readAtomsFromFile(gui->getAtomFilepath(), gui->getIncludeHetatm());}

  catch (const ExceptIllegalFileExtension& e){
    displayErrorMessage(103);
    successful_import = false;
  }
  catch (const ExceptInvalidInputFile& e){
    displayErrorMessage(102);
    successful_import = false;
  }

  gui->displayAtomList(current_calculation->generateAtomList()); // update gui

  return successful_import;
}

// default function call: transfer data from GUI to Model
bool Ctrl::runCalculation(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  // PARAMETERS
  // save parameters in model
  if(!current_calculation->setParameters(
      gui->getAtomFilepath(),
      gui->getOutputDir(),
      gui->getIncludeHetatm(),
      gui->getAnalyzeUnitCell(),
      gui->getCalcSurfaceAreas(),
      gui->getProbeMode(),
      gui->getProbe1Radius(),
      gui->getProbe2Radius(),
      gui->getGridsize(),
      gui->getDepth(),
      gui->getMakeReport(),
      gui->getMakeSurfaceMap(),
      gui->getMakeCavityMaps(),
      gui->generateRadiusMap(),
      gui->getIncludedElements(),
      gui->getMaxRad())){
    return false;
  }

  // CALCULATION
  CalcReportBundle data = current_calculation->generateData();
  calculationDone(data.success);

  // OUTPUT
  clearOutput();
  if(data.success){
    if(!data.cavities.empty()){gui->extDisplayCavityList(data.cavities);}
    std::wstring vol_unit = Symbol::angstrom() + Symbol::cubed();

    notifyUser("Result for ");
    notifyUser(Symbol::generateChemicalFormulaUnicode(data.chemical_formula));
    notifyUser("\nElapsed time: " + std::to_string(data.getTime()) + " s");
    
    notifyUser("\n<VOLUME>");
    notifyUser("\nVan der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " ");
    notifyUser(vol_unit);
    notifyUser("\nProbe inaccessible volume: " + std::to_string(data.volumes[0b00000101]) + " ");
    notifyUser(vol_unit);
    std::string prefix = data.probe_mode? "Small p" : "P";
    notifyUser("\n"+ prefix +"robe core volume: " + std::to_string(data.volumes[0b00001001]) + " ");
    notifyUser(vol_unit);
    notifyUser("\n"+ prefix +"robe shell volume: " + std::to_string(data.volumes[0b00010001]) + " ");
    notifyUser(vol_unit);
    if(data.probe_mode){
      notifyUser("\nLarge probe core volume: " + std::to_string(data.volumes[0b00100001]) + " ");
      notifyUser(vol_unit);
      notifyUser("\nLarge probe shell volume: " + std::to_string(data.volumes[0b01000001]) + " ");
      notifyUser(vol_unit);
    }
    if(data.calc_surface_areas){
      std::wstring surf_unit = Symbol::angstrom() + Symbol::squared();
      notifyUser("\n<SURFACE>");
      notifyUser("\nVan der Waals surface: " + std::to_string(data.getSurfVdw()) + " ");
      notifyUser(surf_unit);
      if(data.probe_mode){
        notifyUser("\nMolecular surface: " + std::to_string(data.getSurfMolecular()) + " ");
        notifyUser(surf_unit);
      }
      notifyUser("\nProbe excluded surface: " + std::to_string(data.getSurfProbeExcluded()) + " ");
      notifyUser(surf_unit);
      notifyUser("\nProbe accessible surface: " + std::to_string(data.getSurfProbeAccessible()) + " ");
      notifyUser(surf_unit);
    }
  }
  else{
    displayErrorMessage(200);
  }
  updateStatus("Calculation done.");
  return data.success;
}

void Ctrl::clearOutput(){
  if (_to_gui) {
    gui->extClearOutputText();
    gui->extClearOutputGrid();
  }
}

void Ctrl::notifyUser(std::string str){
  if (_to_gui){
    gui->extAppendOutput(str);
  }
  else {
    std::cout << str;
  }
}

void Ctrl::notifyUser(std::wstring wstr){
  if (_to_gui){
    gui->extAppendOutputW(wstr);
  }
  else {
    std::cout << wstr;
  }
}

// TODO remove if obselete
void Ctrl::prepareOutput(std::string atomFilePath){
  if(!current_calculation->createOutputFolder(fileName(atomFilePath))){
    displayErrorMessage(301);  
  }
}

void Ctrl::updateStatus(const std::string str){
  if (_to_gui) {
    gui->extSetStatus(str);
  }
  else {
    std::cout << str << std::endl;
  }
}

void Ctrl::updateProgressBar(const int percentage){
  assert (percentage <= 100);
  if (_to_gui) {
    gui->extSetProgressBar(percentage);
  }
  else {
    std::cout << std::to_string(percentage) + "\%"  << std::endl;
  }
}

void Ctrl::exportReport(std::string path){
  current_calculation->createReport(path);
}

void Ctrl::exportReport(){
  current_calculation->createReport();
}

void Ctrl::exportSurfaceMap(const std::string path, bool cavities){
  if(cavities){
    current_calculation->writeCavitiesMaps(path);
  }
  else{
    current_calculation->writeTotalSurfaceMap(path);
  }
}

void Ctrl::exportSurfaceMap(bool cavities){
  if(cavities){
    current_calculation->writeCavitiesMaps();
  }
  else{
    current_calculation->writeTotalSurfaceMap();
  }
}

////////////////////////
// CALCULATION STATUS //
////////////////////////

void Ctrl::newCalculation(){
  _calculation_finished = false;
}

void Ctrl::calculationDone(const bool state){
  _calculation_finished = state;
}

bool Ctrl::isCalculationDone(){
  return _calculation_finished;
}

////////////////////
// ERROR MESSAGES //
////////////////////

static const std::map<int, std::string> s_error_codes = {
  {0, "Unidentified error code."}, // no user should ever see this
  // 1xx: Invalid Input
  {101, "Invalid radius definition file. Please select a valid file or set radii manually."},
  {102, "Invalid structure file. Please select a valid file."},
  {103, "Invalid file format. Please make sure that the input files have the correct file extensions."},
  {104, "Invalid probe radius input. The large probe must have a larger radius than the small probe."},
  // 11x: unit cell files
  {111, "Space group not found. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {112, "Invalid unit cell parameters. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {113, "Space group or symmetry not found. Check the structure and space group files or untick the Unit Cell Analysis tickbox"},
  // 2xx: Issue during Calculation
  {200, "Calculation failed!"}, // unspecific error
  {201, "Total number of cavities (255) exceeded. Consider changing the probe size. Calculation will proceed."},
  // 3xx: Issue with Output
  {301, "New output folder could not be created. The output file(s) will be created in the program folder."},
  {302, "Data missing to export file. Calculation may be still running or has not been started."}
};

void Ctrl::displayErrorMessage(const int error_code){
  if (_to_gui){
    gui->extOpenErrorDialog(error_code, getErrorMessage(error_code));
  }
  else{
    printErrorMessage(error_code);
  }
}

void Ctrl::printErrorMessage(const int error_code){
  std::cout << getErrorMessage(error_code) << std::endl;
}

std::string Ctrl::getErrorMessage(const int error_code){
  if (s_error_codes.find(error_code) == s_error_codes.end()) {
    return s_error_codes.find(0)->second;
  }
  else {
    return s_error_codes.find(error_code)->second;
  }
}
