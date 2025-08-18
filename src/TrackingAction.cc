#include "TrackingAction.hh"


TrackingAction::TrackingAction()
{

}

TrackingAction::~TrackingAction()
{
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{       
    // Define Primary particle information
    G4bool isPrimary = (track->GetParentID() == 0);
    
    G4VPhysicalVolume* vol = track->GetVolume();

    const G4VTouchable* touchable = track->GetTouchable();
    G4int copyNo = touchable ? touchable->GetCopyNumber() : -1;

    auto* info = new TrackingInfo(isPrimary,vol,copyNo);
    const_cast<G4Track*>(track)->SetUserInformation(info);
}
