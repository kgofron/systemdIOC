#!/bin/bash

# Run the IOC with the current user context to avoid privilege issues
# This script ensures the IOC runs with the same user as the terminal

echo "Starting systemd IOC with user context..."
echo "User: $(whoami)"
echo "UID: $(id -u)"
echo "GID: $(id -g)"

# Set environment variables for D-Bus
export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$(id -u)/bus"

# Run the IOC
exec ./st.cmd 