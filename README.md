# Neutron–Silicon Detector Simulation (Geant4)

This repository contains a Geant4-based simulation of neutrons interacting with a stack of silicon detectors. It is set up for both interactive visualization and batch runs, and records detailed information about energy deposition, primary interactions, and secondary particles to ROOT files.

---

## Features

- **Geometry**
  - Vacuum (Galactic) world: 50 cm × 50 cm × 50 cm.
  - Stack of 50 silicon detectors (`G4_Si`), each:
    - Lateral size: 1 cm × 1 cm.
    - Thickness: 1 mm.
    - Positioned along +Z, starting at 5 cm:  
      `z = 5 cm + i * (detThickness)` for `i = 0…49`.
  - Silicon detectors are visible as red wireframe boxes in the visualization.

- **Physics**
  - Uses the **QGSP_BERT_HP** physics list:
    - High-precision neutron physics.
    - Hadronic and electromagnetic interactions suitable for neutron transport and silicon recoil studies.

- **Primary Neutron Source**
  - Neutron gun implemented via `PrimaryGenerator`.
  - Source type configurable at runtime:
    - `"DD"`: monoenergetic 2.45 MeV neutrons (typical D–D source).
    - `"AmBe"`: sampled from an AmBe spectrum CSV file.
    - `"CR"`: sampled from a cosmic-ray–like spectrum CSV file.
  - For spectrum-based sources:
    - CSV files read as:  
      `energy,weight` per line.
    - Weights are normalized to a cumulative distribution function (CDF) for sampling.
  - Primary position:
    - Uniformly random (x, y) in the detector plane:
      - `x, y ∈ [-0.5 * detSize, 0.5 * detSize]` where `detSize = 1 cm`.
      - `z = 0` (just before the detector stack).

- **User Actions & Threading**
  - `ActionInitialization` registers:
    - `PrimaryGenerator`
    - `RunAction`
    - `SteppingAction`
    - `TrackingAction`
    - `StackingAction`
  - **Multithreading**:
    - Uses `G4MTRunManager` when built with `G4MULTITHREADED`.
    - Each worker thread gets a different random seed:  
      `seed = time(nullptr) + threadId`.
    - Master thread also gets its own seed.

- **Stacking**
  - `StackingAction` currently allows all tracks to proceed (`fUrgent`).
  - Commented-out code shows where you can kill certain secondary species (electrons, positrons, gammas) if desired.

---

## Sensitive Detector & Scoring

### SensitiveDetector

- The silicon logical volume is made sensitive via `ConstructSDandField`.
- For each step in a silicon volume:
  - Retrieves event ID and global time.
  - Records:
    - Particle name.
    - Creator process (or `"primary"` if none).
    - Copy number of the silicon volume.
    - 3D position of the step (`x, y, z`).
    - Kinetic energy at that step.
    - Energy deposited in that step.
  - Accumulates total energy deposited in the event:
    - `fTotalEnergyDeposited += stepEdep`.
    - Tracks the copy number where interaction occurred (`fInteractionCopyNo`).

- At end of event:
  - If any energy was deposited:
    - Writes total deposited energy to histogram.
    - Flags event as having an interaction and stores:
      - Total deposited energy.
      - Copy number of the interaction.
  - If no energy was deposited:
    - Writes zeros to the corresponding ntuple entry.
  - Provides a global printout of total deposited energy per event.

### SteppingAction & TrackingAction

- `TrackingAction`:
  - For every new track:
    - Attaches a `TrackingInfo` object.
    - Stores:
      - Whether it is a primary (`parentID == 0`).
      - Birth volume.
      - Copy number of birth volume.

- `SteppingAction` (for primaries):
  - For each step of the primary neutron:
    - Accesses the process that defined the step.
    - If the process is a physical interaction (not `Transportation` / `NoProcess`):
      - Logs:
        - Interaction process name (e.g., `hadElastic`).
        - Position of interaction.
        - Energy loss (`dE = E_pre - E_post`).
      - Records interaction details in the “Primaries” ntuple:
        - Has interacted flag.
        - Total energy deposit.
        - Interaction copy number.
        - Interaction type (process name).
        - ΔKE for the step.
        - Primary energy.

  - **First interaction secondary analysis**:
    - On the first interaction of the primary (when secondaries are created and `HasInteracted()` is still false):
      - For elastic scattering (`hadElastic`):
        - Searches secondaries for silicon recoil (`"Si28"`, `"Si29"`, …).
        - If found:
          - Records recoil kinetic energy as secondary data.
      - If no explicit silicon recoil is found:
        - Sums kinetic energy of all non-EM secondaries and non-neutrinos to approximate fragment kinetic energy.

    - These are written to the “Secondaries” ntuple:
      - Event ID.
      - Copy number.
      - Creator process.
      - Secondary particle name.
      - Kinetic energy of each recorded secondary.
      - Total KE of recoil/fragment system.

  - **Track killing policy**:
    - For primaries:
      - Once a primary has interacted in a given silicon copy (birth volume copy), if it attempts to leave that volume, the track is terminated (`fStopAndKill`).
    - For secondaries:
      - If a secondary tries to leave its birth copy volume, the track is killed.

---

## Data Output

The simulation uses `G4AnalysisManager` to write data to ROOT files.

### Output Files

- One ROOT file per run:
  - File name: `output<runID>.root`
  - E.g. `output0.root`, `output1.root`, etc.
- Ntuple merging is enabled for multithreaded runs.

### Histograms

Currently defined in `RunAction`:

- **H2: `Edep_xy`** (index 0)
  - Title: `"Energy deposition map in XY"`.
  - Axes:
    - X: `[-0.5 cm, 0.5 cm]`
    - Y: `[-0.5 cm, 0.5 cm]`
  - Weighted by deposited energy in MeV.
  - Filled whenever there is non-zero energy deposited in a step inside the sensitive detector.

> Note: Some 1D histograms (e.g. total energy deposition, neutron spectrum) are present in comments and can be re-enabled as needed.

### Ntuples

Three ntuples are defined:

1. **`Particles`** (ID 0) — all hits in the sensitive detector  
   Columns:
   1. `iEvent` – event ID.
   2. `copyNo` – detector copy number (0–49).
   3. `fX` – hit X position (mm).
   4. `fY` – hit Y position (mm).
   5. `fZ` – hit Z position (mm).
   6. `fGlobalTime` – global time of hit.
   7. `fParticleName` – particle name (`neutron`, `gamma`, `Si28`, etc.).
   8. `fCreatorProcess` – name of process creating the track (`primary`, `neutronInelastic`, …).
   9. `fEDep` – energy deposited in this step (MeV).
   10. `fKE` – kinetic energy of the track at this step.

2. **`Primaries`** (ID 1) — per-event summary of primary interaction  
   Columns:
   1. `fHasInteracted` – 1 if any energy deposited in event, else 0.
   2. `fTotalEnergyDep` – total energy deposition in all sensitive volumes (MeV).
   3. `fInteractionCopyNo` – copy number where interaction occurred (`-1` if no interaction).
   4. `fInteractionType` – name of the primary interaction process at the first interaction.
   5. `fDeltaKE` – energy loss of the primary at the interaction step (MeV).
   6. `fPrimaryEnergy` – primary neutron energy (MeV).

3. **`Secondaries`** (ID 2) — immediate secondaries from primary’s first interaction  
   Columns:
   1. `iEvent` – event ID.
   2. `copyNo` – detector copy number where interaction occurred.
   3. `fCreatorProcess` – process that created the secondary (e.g., `neutronInelastic`, `hadElastic`).
   4. `fParticleName` – secondary particle name.
   5. `fKE` – kinetic energy of the secondary (MeV).
   6. `fTotalKE` – total KE of recoil/fragment system for the interaction (MeV).

---

## Runtime Controls

A custom messenger (`Messenger`) provides UI commands to configure the primary source.

### Source Type

Set the neutron source type:

```tcl
/generator/setSource DD
/generator/setSource AmBe
/generator/setSource CR
```

- `"DD"`:
  - No spectrum file used.
  - Fixed energy: 2.45 MeV.
- `"AmBe"`:
  - Loads `AmBeSpectrum.csv` from current working directory.
- `"CR"`:
  - Loads `EXPACsNeutrons.csv` from current working directory and uses `fEmin`, `fEmax` as limits.

### Energy Range (for CR source)

Set the minimum and maximum energy (MeV):

```tcl
/generator/setEnergyRange 1.0 10.0
```

- Only effective when source type is `"CR"`:
  - Internally selects an energy within [E_min, E_max] using the loaded CDF.
- If no energies in the requested range are available in the spectrum, returns 0.0 MeV (this will generate a 0-energy particle; you may want to handle this in your macro).

---

## Building

### Prerequisites

- Geant4 (built with:
  - Multithreading support **recommended**.
  - Visualization drivers (e.g. OpenGL) if you want interactive display.
  - G4Analysis support with ROOT backend, or equivalent.)
- C++ compiler (C++11 or later).
- ROOT (for reading the output).

Configure your environment as usual for Geant4:

```bash
source /path/to/geant4-install/bin/geant4.sh
```

### Typical CMake Build

Assuming the project has a standard CMake structure:

```bash
mkdir build
cd build
cmake ..
make -j
```

This will produce an executable (e.g. `nusim`).

---

## Running

### Interactive Mode

Run without arguments to start the interactive session:

```bash
./nusim
```

This will:

- Initialize the visualization manager.
- Execute `vis.mac` by default.
- Open a Geant4 UI session where you can:
  - Rotate/zoom the geometry.
  - Issue commands such as:
    ```tcl
    /run/initialize
    /generator/setSource DD
    /run/beamOn 1000
    ```

### Batch Mode

Provide a macro file as the first argument:

```bash
./nusim run.mac
```

Example content of a simple `run.mac`:

```tcl
/control/verbose 1
/run/verbose 1
/event/verbose 0
/tracking/verbose 0

/run/initialize
/generator/setSource AmBe
/generator/setEnergyRange 1.0 10.0

/run/beamOn 10000
```

The simulation will write ROOT output files named `output<runID>.root` in the current directory.

---

## Project Structure Overview

Key classes and responsibilities:

- `nusim.cc`
  - Main program.
  - Sets up run manager (MT or single-thread).
  - Registers physics, detector, and actions.
  - Initializes visualization and UI / macro execution.

- `DetectorConstruction`
  - Defines world and silicon detector geometry.
  - Assigns `SensitiveDetector` to silicon logical volume.

- `PhysicsList`
  - Inherits from `QGSP_BERT_HP`.

- `ActionInitialization`
  - Registers:
    - `PrimaryGenerator`
    - `RunAction`
    - `SteppingAction`
    - `TrackingAction`
    - `StackingAction`
  - Handles per-thread random seeding.

- `PrimaryGenerator`
  - Configurable neutron source.
  - Sampling from CSV spectra.
  - Random spatial distribution within detector area.
  - UI messenger interface.

- `RunAction`
  - Defines histograms and ntuples.
  - Manages ROOT file open/close per run.

- `SensitiveDetector`
  - Records hit-level energy deposition and track information.
  - Aggregates total energy per event and interaction summary.

- `TrackingAction` & `TrackingInfo`
  - Attach and manage per-track meta-information (primary/secondary, birth volume, copy number, first interaction).

- `SteppingAction`
  - Analyses primary neutron interactions.
  - Tracks energy loss and secondary production at first interaction.
  - Manages track termination rules.

- `StackingAction`
  - (Currently) allows all tracks; can be extended to kill selected secondaries.

---

## Notes & Extensions

- **Spectrum Files**:
  - Ensure `AmBeSpectrum.csv` and `EXPACsNeutrons.csv` are available in the runtime directory if using `"AmBe"` or `"CR"` sources.
  - CSV format:  
    ```text
    energy,weight
    1.0,0.5
    2.0,1.0
    ...
    ```

- **Customizing Geometry**:
  - Detector number, dimensions, and spacing can be changed in `DetectorConstruction`.

- **Additional Scoring / Output**:
  - More histograms and ntuples can be enabled or added in `RunAction`, `SensitiveDetector`, and `SteppingAction`.

- **Track Filtering**:
  - Use `StackingAction` to selectively kill unwanted secondaries (e.g., EM background) to speed up simulations.
