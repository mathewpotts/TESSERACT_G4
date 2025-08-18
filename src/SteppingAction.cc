#include "SteppingAction.hh"

#include "G4VProcess.hh"

SteppingAction::SteppingAction()
{
}

SteppingAction::~SteppingAction()
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    auto* info = static_cast<TrackingInfo*>(track->GetUserInformation());
    if (!info) return;

    // Get pre/post step copy number from touchable
    G4int preCopyNo = step->GetPreStepPoint()->GetTouchable()->GetCopyNumber();
    G4int postCopyNo = step->GetPostStepPoint()->GetTouchable()->GetCopyNumber();
    G4int birthCopyNo = info->GetCopyNo();

    // Example: check if a specific process caused the step
    G4StepPoint* preStepPoint = step->GetPreStepPoint();
    G4StepPoint* postStepPoint = step->GetPostStepPoint();
    const G4VProcess* process = postStepPoint->GetProcessDefinedStep();
    if (process) {
        G4String procName = process->GetProcessName();
        
        if (procName == "nCapture" || procName == "hadElastic" || procName == "neutronInelastic") {
            G4double E_pre  = preStepPoint->GetKineticEnergy();
            G4double E_post = postStepPoint->GetKineticEnergy();
            G4double deltaE = E_pre - E_post;
            G4ThreeVector position = preStepPoint->GetPosition();
            
                G4cout << "SteppingAction::UserSteppingAction Neutron interaction (" << procName << ") at "
                       << position << ", dE = " << deltaE / keV << " keV" << G4endl;
        }
    }
    
    if (info->IsPrimary()) {
        if (!info->HasInteracted()) {
            // Check if secondaries are created in this step
            const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
            if (secondaries && !secondaries->empty()) {
                G4int copyNo = step->GetPreStepPoint()->GetTouchable()->GetCopyNumber();
                info->SetHasInteracted(copyNo);
                G4cout << "SteppingAction::UserSteppingAction Primary track " << track->GetTrackID() << " created secondaries and interacted in CopyNo: " << copyNo << G4endl;
            }
        }
        // kill primary if leaving birth volume after interaction
        G4int InteractionCopyNo = info->GetInteractionCopyNo();
        if (info->HasInteracted() &&
            preCopyNo == InteractionCopyNo &&
            postCopyNo != InteractionCopyNo) {
            G4cout << "SteppingAction::UserSteppingAction Killing primary track " << track->GetTrackID() << " after interaction leaving birth volume." << G4endl;
            track->SetTrackStatus(fStopAndKill);
        }
    } else if (!info->IsPrimary()
               && (preCopyNo == birthCopyNo && postCopyNo != birthCopyNo)) {
        G4cout << "SteppingAction::UserSteppingAction Killing secondary track " << track->GetTrackID()
               << " (" << track->GetDefinition()->GetParticleName()
               << ") attempting to leave birth volume (CopyNo " << birthCopyNo
               << "). Next volume CopyNo: " << postCopyNo << G4endl;
        track->SetTrackStatus(fStopAndKill);
    }
}

