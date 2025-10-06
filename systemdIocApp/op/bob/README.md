# CSS Phoebus Systemd Service Control Screen

This directory contains a CSS Phoebus .bob screen for controlling systemd services from EPICS.

## Files

- `systemd_service.bob` - The main Phoebus screen file for systemd service control

## Usage

### 1. Load the Screen in CSS Phoebus

Open CSS Phoebus and load the screen:
```
File -> Open -> Browse to systemd_service.bob
```

### 2. Configure PV Prefixes

When loading the screen, you'll be prompted to enter PV prefixes. Use the same prefixes as defined in your `st.cmd` file:

- **P**: The PV prefix (e.g., `serval:`)
- **R**: The record suffix (e.g., `service:`)

### 3. Screen Features

The screen provides:

#### Service Information
- **Service Name**: Displays the name of the systemd service being controlled
- **Status**: Real-time status of the service with color coding:
  - 🟢 **Green**: Service is running
  - 🔴 **Red**: Service is stopped
  - 🟠 **Orange**: Service is starting/stopping
  - ⚫ **Gray**: Service not found
  - ⚫ **Black**: Unknown status

#### Control Buttons
- **Start/Stop Button**: 
  - Green "Start" button when service is stopped
  - Red "Stop" button when service is running
  - Includes confirmation dialog before action
- **Reset Failed Button**: 
  - Orange button to reset failed service state
  - Includes confirmation dialog before action

#### Auto-refresh
- Status updates automatically every 5 seconds
- Manual refresh available through CSS Phoebus interface

### 4. PV Mapping

The screen uses the following EPICS PVs:

| PV Name | Type | Description |
|---------|------|-------------|
| `$(P)$(R)ServiceName` | String | Name of the systemd service |
| `$(P)$(R)Status` | String | Current service status |
| `$(P)$(R)Start` | Binary | Start (1) or Stop (0) the service |
| `$(P)$(R)ResetFailed` | Binary | Reset failed state (1) |

### 5. Example Configuration

If your `st.cmd` contains:
```bash
setServiceName("apache2.service")
dbLoadRecords("db/systemd.db", "P=web:,R=server:,SERVICE=apache2.service")
```

Then when loading the screen, use:
- **P**: `web:`
- **R**: `server:`

This will create PVs like:
- `web:server:ServiceName`
- `web:server:Status`
- `web:server:Start`
- `web:server:ResetFailed`

### 6. Customization

You can modify the `systemd_service.bob` file to:
- Change colors and fonts
- Add additional controls
- Modify the layout
- Add more status information

### 7. Troubleshooting

**Screen not loading**: Check that CSS Phoebus is properly installed and the .bob file is valid XML.

**PVs not connecting**: Verify that:
- The IOC is running
- PV prefixes match your `st.cmd` configuration
- The systemd service name is correctly set

**Buttons not working**: Ensure that:
- The IOC has proper permissions to control systemd services
- Polkit rules are configured correctly
- The service name is valid

## Security Notes

- The screen includes confirmation dialogs for all control actions
- Ensure proper user permissions are configured for systemd service control
- Consider restricting access to this screen based on user roles
