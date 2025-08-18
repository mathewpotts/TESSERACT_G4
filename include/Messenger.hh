#ifndef MESSENGER_H
#define MESSENGER_H 1

#include "G4UIcmdWithAString.hh"
#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"
#include "G4RunManager.hh"

#include "PrimaryGenerator.hh"

// Forward delcare class
class PrimaryGenerator;

class Messenger : public G4UImessenger {
public:
    Messenger(PrimaryGenerator* generator);
    virtual ~Messenger();

    virtual void SetNewValue(G4UIcommand* command, G4String newValue);

private:
    PrimaryGenerator* fPrimaryGenerator;

    G4UIcmdWithAString* neuSourceCmd;
};

#endif
