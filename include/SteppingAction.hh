#ifndef SteppingAction_hh
#define SteppingAction_hh

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"    // for fStopAndKill
#include "G4UserSteppingAction.hh"  // usually included by your header
#include "G4SystemOfUnits.hh"  // if you use units
#include "G4ThreeVector.hh"    // if you use vectors
#include "G4ios.hh"            // for G4cout, G4endl
#include "G4AnalysisManager.hh"
#include "globals.hh"

#include "TrackingInfo.hh"

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction();
    ~SteppingAction();

    virtual void UserSteppingAction(const G4Step* step) override;
    
};

#endif
