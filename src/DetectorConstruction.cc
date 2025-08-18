#include "DetectorConstruction.hh"

DetectorConstruction::DetectorConstruction()
{

}

DetectorConstruction::~DetectorConstruction()
{

}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    G4bool checkOverlaps = true;

    G4NistManager *nist = G4NistManager::Instance();
    G4Material *worldMat = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material *detMat = nist->FindOrBuildMaterial("G4_Si");
    
    G4double xWorld = 50. * cm;
    G4double yWorld = 50. * cm;
    G4double zWorld = 50. * cm;

    G4Box *solidWorld = new G4Box("solidWorld",
                                  0.5 * xWorld,
                                  0.5 * yWorld,
                                  0.5 * zWorld);
    
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld,
                                                      worldMat,
                                                      "logicWorld");

    G4VPhysicalVolume *physWorld = new G4PVPlacement(
    0,                      // no rotation
    G4ThreeVector(0., 0., 0.),    // position
    logicWorld,                   // logical volume
    "physWorld",                  // name
    0,                      // mother volume
    false,                        // no boolean operations
    0,                            // copy number
    checkOverlaps);                 // overlap check
                                                     

    G4double detThickness = 1. * mm;
    detSize = 1. * cm;

    G4Box *solidDet = new G4Box("solidDet", 0.5 * detSize,
                                0.5 * detSize,
                                0.5 * detThickness);
    logicDet = new G4LogicalVolume(solidDet,
                                   detMat,
                                   "logicDet");

    // Place 10 detectors stacked
    for (G4int i = 0; i < 50; i++){
        G4VPhysicalVolume *physDet = new G4PVPlacement(
        0,
        G4ThreeVector(0.,0.,5*cm + i*(detThickness)),
        logicDet,
        "physDet",
        logicWorld,
        false,
        i,
        checkOverlaps);
    }
    
    
    G4VisAttributes *detVisAtt = new G4VisAttributes(G4Color(1.0,0.0,0.0,0.5));
    //detVisAtt->SetForceSolid(true);
    detVisAtt->SetForceWireframe(true);
    detVisAtt->SetVisibility(true);
    logicDet->SetVisAttributes(detVisAtt);
    
    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    SensitiveDetector *sensDet = new SensitiveDetector("Sensitive Detector");
    logicDet->SetSensitiveDetector(sensDet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);
}

