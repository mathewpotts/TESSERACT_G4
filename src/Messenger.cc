#include "Messenger.hh"

Messenger::Messenger(PrimaryGenerator* generator) : fPrimaryGenerator(generator)
{
    neuSourceCmd = new G4UIcmdWithAString("/generator/setSource", this);
    neuSourceCmd->SetGuidance("Set the name of neutron gun source");
    neuSourceCmd->SetParameterName("Source Name", false);
    neuSourceCmd->SetDefaultValue("DD");

    energyRangeCmd = new G4UIcommand("/generator/setEnergyRange", this);
    energyRangeCmd->SetGuidance("Set the energy range (min max) in MeV");
    
    auto energyMin = new G4UIparameter("Emin", 'd', false);
    energyMin->SetGuidance("Minimum energy (MeV)");
    energyRangeCmd->SetParameter(energyMin);
    
    auto energyMax = new G4UIparameter("Emax", 'd', false);
    energyMax->SetGuidance("Maximum energy (MeV)");
    energyRangeCmd->SetParameter(energyMax);
}

Messenger::~Messenger()
{
    delete neuSourceCmd;
    delete energyRangeCmd;

}

void Messenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == neuSourceCmd) {
        fPrimaryGenerator->SetSourceType(newValue);
    }
    if (command == energyRangeCmd) {
        std::istringstream iss(newValue);
        G4double emin, emax;
        iss >> emin >> emax;
        fPrimaryGenerator->SetEnergyRange(emin, emax);
    }

}
