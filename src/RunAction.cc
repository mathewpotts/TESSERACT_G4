#include "RunAction.hh"

RunAction::RunAction()
{
    G4AnalysisManager *analManager = G4AnalysisManager::Instance();

    analManager->CreateH1("Edep", "Energy deposit", 1000, 0.,10. * keV);
    analManager->SetH1Title(0, "Neutron Energy deposited");
    analManager->SetH1XAxisTitle(0, "Energy (MeV)");
    analManager->SetH1YAxisTitle(0, "Counts");

    analManager->CreateH1("NeuSpec", "Neutron Spectrum", 1000, 0., 10 * eV);
    analManager->SetH1Title(1, "Neutron Spectrum");
    analManager->SetH1XAxisTitle(1, "Energy (MeV)");
    analManager->SetH1YAxisTitle(1, "Counts");
    
    analManager->CreateH2("Edep_xy","XY EnergyDeposition",
                          100, -0.5*cm, 0.5*cm,
                          100, -0.5*cm, 0.5*cm);
    analManager->SetH2Title(0, "Energy deposition map in XY");
    analManager->SetH2XAxisTitle(0, "X position (mm)");
    analManager->SetH2YAxisTitle(0, "Y position (mm)");
    analManager->SetH2ZAxisTitle(0, "Deposited Energy (MeV)");

    analManager->CreateNtuple("Particles","Particles");
    analManager->CreateNtupleIColumn("iEvent");
    analManager->CreateNtupleIColumn("copyNo");
    analManager->CreateNtupleDColumn("fX");
    analManager->CreateNtupleDColumn("fY");
    analManager->CreateNtupleDColumn("fZ");
    analManager->CreateNtupleDColumn("fGlobalTime");
    analManager->CreateNtupleSColumn("fParticleName");
    analManager->CreateNtupleSColumn("fCreatorProcess");
    analManager->CreateNtupleDColumn("fEDep");
    analManager->FinishNtuple(0);

    analManager->CreateNtuple("Interacted","Interacted");
    analManager->CreateNtupleIColumn("fHasInteracted");
    analManager->CreateNtupleDColumn("fTotalEnergyDep");
    analManager->CreateNtupleIColumn("fInteractionCopyNo");
    analManager->CreateNtupleDColumn("fPrimaryEnergy");
    analManager->FinishNtuple(1);
}

RunAction::~RunAction()
{

}

void RunAction::BeginOfRunAction(const G4Run *run)
{
    G4AnalysisManager *analManager = G4AnalysisManager::Instance();

    G4int runID = run->GetRunID();

    std::stringstream strRunID;
    strRunID << runID;

    analManager->SetNtupleMerging(true);
    analManager->OpenFile("output" + strRunID.str() + ".root");

}

void RunAction::EndOfRunAction(const G4Run *run)
{
    G4AnalysisManager *analManager = G4AnalysisManager::Instance();

    analManager->Write();

    analManager->CloseFile();

    G4int runID = run->GetRunID();
    G4cout << "Finishing run " << runID << G4endl;
}
