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
    G4ParticleDefinition* particle = track->GetDefinition();
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

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    
    if (process && info->IsPrimary()) {
        G4String procName = process->GetProcessName();
        
        if (procName != "Transportation" && procName != "NoProcess") {
            G4double E_pre  = preStepPoint->GetKineticEnergy();
            G4double E_post = postStepPoint->GetKineticEnergy();
            G4double deltaE = E_pre - E_post;
            G4ThreeVector position = preStepPoint->GetPosition();
            
            G4cout << "SteppingAction::UserSteppingAction Neutron interaction (" << procName << ") at "
                   << position << ", dE = " << deltaE / keV << " keV" << G4endl;

            // Fill the primary interaction type
            analysisManager->FillNtupleSColumn(1, 3, procName);
            analysisManager->FillNtupleDColumn(1, 4, deltaE / MeV);

            // Only do this on the FIRST interaction of the primary
            // (we can key off info->HasInteracted(), which you set below)
            if (!info->HasInteracted()) {

                const auto* secondaries = step->GetSecondaryInCurrentStep();
                G4double recoilOrFragKE = 0.0;

                if (secondaries && !secondaries->empty()) {
                    const G4Track* recoilSi = nullptr;

                    // 1) try to find a Si recoil only for elastic scattering
                    if (procName == "hadElastic") {
                        for (const G4Track* sec : *secondaries) {
                            G4String secName = sec->GetDefinition()->GetParticleName();
                            if (secName.contains("Si")) { // e.g. "Si28", "Si29", ...
                                recoilSi = sec;
                                break;
                            }
                        }
                    }

                    if (recoilSi) {
                        // Case 1: we have an explicit Si recoil
                        recoilOrFragKE = recoilSi->GetKineticEnergy();
                        G4cout << "SteppingAction::UserSteppingAction Found Si recoil: KE = "
                               << recoilOrFragKE / keV << " keV" << G4endl;
                    } else {
                        // Case 2: no explicit Si recoil – sum KE of nucleus+fragments
                        G4double sumFragKE = 0.0;

                        for (const G4Track* sec : *secondaries) {
                            auto* def = sec->GetDefinition();
                            G4String secName = def->GetParticleName();
                            G4String secType = def->GetParticleType();

                            // Skip pure EM secondaries
                            if (secName == "gamma" ||
                                secName == "e-"   ||
                                secName == "e+") continue;

                            // Optionally also treat muons as EM:
                            if (secName == "mu+" || secName == "mu-") continue;

                            // Everything else is considered "fragment"
                            bool isFragment = true;
                            
                            /*
                            // Accept "nucleus" type and common fragments
                            bool isFragment = (secType == "nucleus") ||
                                              secName == "proton"   ||
                                              secName == "neutron"  ||
                                              secName == "deuteron" ||
                                              secName == "triton"   ||
                                              secName == "He3"      ||
                                              secName == "alpha";
                            */
                            if (isFragment) {
                                sumFragKE += sec->GetKineticEnergy();
                            }
                        }

                        recoilOrFragKE = sumFragKE;
                        G4cout << "SteppingAction::UserSteppingAction No Si recoil found; sum(fragment KE) = "
                               << sumFragKE / keV << " keV" << G4endl;
                    }
                }

                // Store recoil or fragment KE in Interacted ntuple (column index 6)
                analysisManager->FillNtupleDColumn(1, 6, recoilOrFragKE / MeV);
            }
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
               << " (" << particle->GetParticleName()
               << ", KE: " << track->GetKineticEnergy() / keV << " keV" 
               << ") attempting to leave birth volume (CopyNo " << birthCopyNo
               << "). Next volume CopyNo: " << postCopyNo << G4endl;
        track->SetTrackStatus(fStopAndKill);
    }
}
