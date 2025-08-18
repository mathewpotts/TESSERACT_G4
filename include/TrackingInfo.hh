#ifndef TRACKINGINFO_HH
#define TRACKINGINFO_HH

#include "G4VUserTrackInformation.hh"
#include "G4VPhysicalVolume.hh"
#include "globals.hh"

class TrackingInfo : public G4VUserTrackInformation {
public:
    TrackingInfo(G4bool isPrimary = false,
                 G4VPhysicalVolume* birthVol = nullptr,
                 G4int copyNo = -1)
        : fIsPrimary(isPrimary), fBirthVolume(birthVol), fCopyNo(copyNo), fHasInteracted(false){}
    
    G4bool IsPrimary() const { return fIsPrimary; }
    G4VPhysicalVolume* GetBirthVolume() const { return fBirthVolume; }
    G4int GetCopyNo() const {return fCopyNo;}

    bool HasInteracted() const { return fHasInteracted; }
    void SetHasInteracted(G4int copyno) {
        fHasInteracted = true;
        fInteractionCopyNo = copyno;
    }
    G4int GetInteractionCopyNo() const { return fInteractionCopyNo; }

private:
    G4bool fIsPrimary;
    G4VPhysicalVolume* fBirthVolume;
    G4int fCopyNo;
    bool fHasInteracted;
    G4int fInteractionCopyNo = -1;
};

#endif
