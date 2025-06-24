#!/bin/bash

# Exit on error
set -e

echo "Building systemd_control..."
g++ -std=c++17 -Wall -Wextra systemd_control.cpp -o systemd_control -lsystemd

echo "Setting up permissions..."
sudo chown root:root systemd_control
sudo chmod u+s systemd_control

echo "Running systemd_control..."
./systemd_control

echo "Done!"
