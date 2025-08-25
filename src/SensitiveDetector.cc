#include "SensitiveDetector.hh"

SensitiveDetector::SensitiveDetector(G4String name) : G4VSensitiveDetector(name)
{
    fTotalEnergyDeposited = 0.;
}

SensitiveDetector::~SensitiveDetector()
{
}

void SensitiveDetector::Initialize(G4HCofThisEvent *)
{
    fTotalEnergyDeposited = 0.;
    fInteractionCopyNo = -1;
}

void SensitiveDetector::EndOfEvent(G4HCofThisEvent *)
{
    G4AnalysisManager *analMan = G4AnalysisManager::Instance();
    
    if (fTotalEnergyDeposited > 0) {
            
        G4cout << "SensitiveDetector::EndOfEvent Depostied Energy: " << fTotalEnergyDeposited / MeV << " MeV" << G4endl;
        
        analMan->FillH1(0,fTotalEnergyDeposited);

        analMan->FillNtupleIColumn(1, 0, 1);
        analMan->FillNtupleDColumn(1, 1, fTotalEnergyDeposited);
        analMan->FillNtupleIColumn(1, 2, fInteractionCopyNo);
    } else { 
        analMan->FillNtupleIColumn(1, 0, 0);
        analMan->FillNtupleDColumn(1, 1, 0.);
        analMan->FillNtupleIColumn(1, 2, -1);
    }

    analMan->AddNtupleRow(1);
}

G4bool SensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    G4AnalysisManager *analMan = G4AnalysisManager::Instance();
    G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    
    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
    G4double fGlobalTime = preStepPoint->GetGlobalTime();
    G4ThreeVector posParticle = preStepPoint->GetPosition();
    
    G4double fEnergyDeposited = aStep->GetTotalEnergyDeposit();

    // Define track
    G4Track *track = aStep->GetTrack();

    // calling TrackingInfo class to get information
    auto* info = static_cast<TrackingInfo*>(track->GetUserInformation());

    // Get the particle name
    G4String particle = track->GetDefinition()->GetParticleName();

    // Get the process that created this track
    const G4VProcess* creator = track->GetCreatorProcess();
    G4String creatorName = "primary"; // default if null
    if (creator) {
        creatorName = creator->GetProcessName();
    }

    // Get the copyNo
    const G4VTouchable *touchable = aStep->GetPreStepPoint()->GetTouchable();
    G4int copyNo = touchable->GetCopyNumber();

    // Get kinetic energy
    G4double fKE = track->GetKineticEnergy();
    
    analMan->FillNtupleIColumn(0, 0, eventID);
    analMan->FillNtupleIColumn(0, 1, copyNo);
    analMan->FillNtupleDColumn(0, 2, posParticle[0]);
    analMan->FillNtupleDColumn(0, 3, posParticle[1]);
    analMan->FillNtupleDColumn(0, 4, posParticle[2]);
    analMan->FillNtupleDColumn(0, 5, fGlobalTime);
    analMan->FillNtupleSColumn(0, 6, particle);
    analMan->FillNtupleSColumn(0, 7, creatorName);
    analMan->FillNtupleDColumn(0, 8, fEnergyDeposited / MeV);
    analMan->FillNtupleDColumn(0, 9, fKE);
    
    
    if (fEnergyDeposited > 0) {
        // Fill 2D histogram with x, y, weighted by energy deposit
        analMan->FillH2(0, posParticle[0], posParticle[1], fEnergyDeposited);
        
        G4cout << "SensitiveDetector::ProcessHits [Hit] Particle: " << particle
               << ", CopyNo: " << copyNo
               << ", Edep: " << fEnergyDeposited / keV << " keV" << G4endl;

        fTotalEnergyDeposited += fEnergyDeposited;
        fInteractionCopyNo = copyNo;
    }

    analMan->AddNtupleRow(0);
    
    return true;
}
