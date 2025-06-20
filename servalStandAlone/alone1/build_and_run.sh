g#!/bin/bash

# Exit on error
set -e

echo "Building get_service_status..."
gcc get_service_status.c -o get_service_status -lsystemd

#echo "Setting up permissions..."
#sudo chown root:root systemd_control
#sudo chmod u+s systemd_control

echo "Running systemd_control..."
./get_service_status

echo "Done!" 
