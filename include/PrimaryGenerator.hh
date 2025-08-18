#ifndef PRIMARYGENERATOR_HH
#define PRIMARYGENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "G4AnalysisManager.hh"

#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "Messenger.hh"

#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <vector>

// Forward declare
class Messenger;

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGenerator(DetectorConstruction* det);
    ~PrimaryGenerator();

    virtual void GeneratePrimaries(G4Event *);

    // Setter for fSourceType
    void SetSourceType(const G4String& type);

private:
    G4ParticleGun *fParticleGun;
    DetectorConstruction* fDetectorConstruction;
    Messenger* fMessenger;
    
    G4double sampleEnergyAmBe();
    G4double EnergyDD();
    G4double EnergyCR(G4double E_min, G4double E_max);
    G4ThreeVector randomPos();
    G4double uniformMinusOneToOne();

    //AmBe spec
    std::vector<G4double> energies;
    std::vector<G4double> weights;
    std::vector<G4double> cdf;
    void LoadSpectrum(const G4String&);
    G4bool fSpectrumLoaded = false;
    
    // Variable to hold source type
    G4String fSourceType;
};


#endif
