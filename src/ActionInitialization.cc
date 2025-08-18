#include "ActionInitialization.hh"

ActionInitialization::ActionInitialization(DetectorConstruction* det) : fDetectorConstruction(det)
{
    
}

ActionInitialization::~ActionInitialization()
{
    
}

void ActionInitialization::BuildForMaster() const
{
    // Master thread can also have its own seed
    G4long seed = time(nullptr);
    G4Random::setTheSeed(seed);
    
    RunAction *runaction = new RunAction();
    SetUserAction(runaction);
}

void ActionInitialization::Build() const
{
    
    // This is called for each worker thread
    G4int tid = G4Threading::G4GetThreadId();
    G4long seed = time(nullptr) + tid;
    
    G4Random::setTheSeed(seed);

    G4cout << "[Seed] Using random seed: " << seed << G4endl;
    
    PrimaryGenerator *generator = new PrimaryGenerator(fDetectorConstruction);
    SteppingAction *stepping = new SteppingAction();
    TrackingAction *tracking = new TrackingAction();
    RunAction *runaction  = new RunAction();
    
    SetUserAction(generator);
    SetUserAction(runaction);
    SetUserAction(stepping);
    SetUserAction(tracking);

}
