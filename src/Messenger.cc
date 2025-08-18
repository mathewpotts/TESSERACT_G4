#include "Messenger.hh"

Messenger::Messenger(PrimaryGenerator* generator) : fPrimaryGenerator(generator)
{
    neuSourceCmd = new G4UIcmdWithAString("/generator/setSource", this);
    neuSourceCmd->SetGuidance("Set the name of neutron gun source");
    neuSourceCmd->SetParameterName("Source Name", false);
    neuSourceCmd->SetDefaultValue("DD");
    
}

Messenger::~Messenger()
{
    delete neuSourceCmd;
}

void Messenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == neuSourceCmd) {
        fPrimaryGenerator->SetSourceType(newValue);
    }

}
