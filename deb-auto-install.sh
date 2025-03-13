#!/bin/bash

# Define version and targets
TARGETS=(
    "ZeunDependencies-debug-static"
    "ZeungineDependencies-static"
    "ZeungineDependencies-debug"
    "ZeungineDependencies"
    "ZeungineHeaders"
    "Zeungine"
    "Zeungine-debug"
    "Zeungine-static"
    "Zeungine-debug-static"
)

# Iterate over targets and run them
for TARGET in "${TARGETS[@]}"; do
    PACKAGE="$TARGET.deb"
    if [[ -f "$PACKAGE" ]]; then
        echo "Installing $PACKAGE"
        sudo dpkg -i $PACKAGE
    else
        echo "Installer $PACKAGE not found."
    fi
done