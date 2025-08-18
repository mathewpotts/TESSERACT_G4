#include "PrimaryGenerator.hh"

PrimaryGenerator::PrimaryGenerator(DetectorConstruction* det) : fDetectorConstruction(det)
{
    fParticleGun = new G4ParticleGun(1);
    fMessenger = new Messenger(this);
    
    //Particle direction
    G4double px = 0.;
    G4double py = 0.;
    G4double pz = 1.;

    G4ThreeVector mom(px,py,pz);

    //Particle Type
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("neutron");

    fParticleGun->SetParticlePosition( randomPos() );
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleEnergy(1. * GeV);
    fParticleGun->SetParticleDefinition(particle);
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun;
}

void PrimaryGenerator::SetSourceType(const G4String& type) {
    fSourceType = type;
    fSpectrumLoaded = false;  // Reset flag if source type changes

    if (!fSpectrumLoaded) {
        if (fSourceType == "AmBe") {
            LoadSpectrum("AmBeSpectrum.csv");
            fSpectrumLoaded = true;
        } else if (fSourceType == "CR") {
            LoadSpectrum("EXPACsNeutrons.csv");
            fSpectrumLoaded = true;
        } else if (fSourceType == "DD") {
            // No spectrum needed for DD
            fSpectrumLoaded = true;
        } else {
            G4Exception("PrimaryGenerator::SetSourceType",
                        "InvalidSourceType", JustWarning,
                        ("Unknown source type: " + fSourceType).c_str());
        }
    }
}

void PrimaryGenerator::LoadSpectrum(const G4String& filename) {
    std::ifstream file(filename);
    G4String line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        G4double energy, weight;
        char comma;
        ss >> energy >> comma >> weight;
        energies.push_back(energy);
        weights.push_back(weight);
    }

    // Normalize weights into CDF
    G4double total = std::accumulate(weights.begin(), weights.end(), 0.0);
    cdf.resize(weights.size());
    std::partial_sum(weights.begin(), weights.end(), cdf.begin());
    for (auto& val : cdf) val /= total;
}

// Sample the energy from AmBe source csv
G4double PrimaryGenerator::sampleEnergyAmBe()
{
    double r = G4UniformRand(); // in [0, 1)
    auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
    size_t index = std::distance(cdf.begin(), it);
    return energies[index];
}


// Energy typical for a DD source
G4double PrimaryGenerator::EnergyDD()
{
    return 2.45 * MeV;
}

// Energy of CR
G4double PrimaryGenerator::EnergyCR(G4double E_min, G4double E_max)
{
    // Find index range corresponding to [E_min, E_max]
    auto it_low = std::lower_bound(energies.begin(), energies.end(), E_min);
    auto it_high = std::upper_bound(energies.begin(), energies.end(), E_max);

    if (it_low == energies.end() || it_low == it_high)
        return 0.0; // or handle error: no energies in this range

    size_t start = std::distance(energies.begin(), it_low);
    size_t end = std::distance(energies.begin(), it_high);

    // Slice the CDF and energies vectors
    std::vector<double> sub_cdf(cdf.begin() + start, cdf.begin() + end);
    std::vector<G4double> sub_energies(energies.begin() + start, energies.begin() + end);

    // Normalize the sliced CDF to go from 0 to 1
    double min_cdf = sub_cdf.front();
    double max_cdf = sub_cdf.back();
    for (auto& val : sub_cdf)
        val = (val - min_cdf) / (max_cdf - min_cdf);

    // Sample using inverse transform method
    double r = G4UniformRand();
    auto it = std::lower_bound(sub_cdf.begin(), sub_cdf.end(), r);
    size_t index = std::distance(sub_cdf.begin(), it);

    return sub_energies[index];
}

G4double PrimaryGenerator::uniformMinusOneToOne()
{
    return 2.0 * G4UniformRand() - 1.0;
}


G4ThreeVector PrimaryGenerator::randomPos()
{
    G4double halfSize = 0.5 * fDetectorConstruction->GetDetectorSize();
    
    G4double x = uniformMinusOneToOne() * halfSize;
    G4double y = uniformMinusOneToOne() * halfSize;
    
    G4ThreeVector pos(x, y, 0.);

    return pos;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    //Set Source Type
    G4double energy = 0.;
    if (fSourceType == "DD") {
        energy = EnergyDD();
    } else if (fSourceType == "AmBe") {
        energy = sampleEnergyAmBe();
    } else if (fSourceType == "CR" ) {
        energy = EnergyCR(1e-8, 1e-5);
    }
    
    fParticleGun->SetParticleEnergy(energy);

    // Fill the primary energy spectrum
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillH1(1, energy);
    analysisManager->FillNtupleDColumn(1, 3, energy);
    
    //Set position
    G4ThreeVector pos = randomPos();
    fParticleGun->SetParticlePosition(pos);

    G4cout << "PrimaryGenerator::GeneratePrimaries Primary neutron E: " << energy / MeV << " MeV" <<G4endl; 
    
    //Create vertex
    fParticleGun->GeneratePrimaryVertex(anEvent);

}
