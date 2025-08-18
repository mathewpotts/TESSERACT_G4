#include <iostream>

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"

#include "PhysicsList.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

int main(int argc, char** argv)
{
    G4UIExecutive *ui; 

    #ifdef G4MULTITHREADED
    G4MTRunManager *runManager = new G4MTRunManager;
    #else
    G4RunManager *runManager = new G4RunManager;
    #endif

    // Physics list
    runManager->SetUserInitialization(new PhysicsList());
    
    // Detector construction
    DetectorConstruction* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);
    
    // Action initialization
    runManager->SetUserInitialization(new ActionInitialization(detector));

    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    } 
    
    
    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if (ui) {
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    } else {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    
    delete visManager;   // Delete visManager *after* UI is done
    delete runManager;   // Delete runManager last

    return 0;
}
