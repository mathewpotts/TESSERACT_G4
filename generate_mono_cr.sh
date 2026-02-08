#!/bin/bash
MAC_FILE="run.mac"

# Backup once
if [ ! -f "${MAC_FILE}.bak" ]; then
    cp "$MAC_FILE" "${MAC_FILE}.bak"
fi

# List of central energies (single value now, not Emin:Emax)
energies=(
    0.01
    0.02
    0.03
    0.06
    0.1
    0.2
    0.3
    0.6
    1.0
    2.0
    3.0
    6.0
    10.0
    20.0
    30.0
    60.0
    100.0
    200.0
    300.0
    600.0
    1000.0
    2000.0
    3000.0
    6000.0
    10000.0
)

for E_CENTER in "${energies[@]}"; do
    # Compute ±10% around the central energy
    E_MIN=$(awk -v e="$E_CENTER" 'BEGIN { printf "%.6g", e*0.9 }')
    E_MAX=$(awk -v e="$E_CENTER" 'BEGIN { printf "%.6g", e*1.1 }')

    echo "Running with central energy ${E_CENTER}, range ${E_MIN} – ${E_MAX}"

    # Edit run.mac
    sed -i "s|^/analysis/h1/create Edep \"Energy deposit\" 1000 .*|/analysis/h1/create Edep \"Energy deposit\" 1000 ${E_MIN} ${E_MAX} MeV|" "$MAC_FILE"

    sed -i "s|^/analysis/h1/create Neuspec \"Neutron Spectrum\" 1000 .*|/analysis/h1/create Neuspec \"Neutron Spectrum\" 1000 ${E_MIN} ${E_MAX} MeV|" "$MAC_FILE"

    sed -i "s|^/generator/setEnergyRange .*|/generator/setEnergyRange ${E_CENTER} ${E_CENTER}|" "$MAC_FILE"

    # Run the simulation
    ./nusim "$MAC_FILE"

    echo "Done: ${E_MIN}–${E_MAX}"
    echo
    #read -p "Press Enter to continue to the next energy..."   # pause here
    mv "output0.root" "../data/cr_${E_CENTER}MeV.root"
done
