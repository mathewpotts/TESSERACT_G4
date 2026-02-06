#include "StackingAction.hh"

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* track)
{
    /*
    auto particle = track->GetDefinition();

    if (particle == G4Electron::ElectronDefinition() ||
        particle == G4Positron::PositronDefinition() ||
        particle == G4Gamma::GammaDefinition()) {
        return fKill;
    }
    */
    return fUrgent;
}
