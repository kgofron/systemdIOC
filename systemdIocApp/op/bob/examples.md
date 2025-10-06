# Example Configurations for Systemd Service Control Screen

This file provides example configurations for using the systemd service control screen with different services.

## Example 1: Apache Web Server

### st.cmd Configuration
```bash
# Set the service name to control Apache
setServiceName("apache2.service")

# Load record instances
dbLoadRecords("db/systemd.db", "P=web:,R=server:,SERVICE=apache2.service")
```

### Screen Configuration
When loading `systemd_service.bob`:
- **P**: `web:`
- **R**: `server:`

### Resulting PVs
- `web:server:ServiceName` → "apache2.service"
- `web:server:Status` → "running", "stopped", etc.
- `web:server:Start` → Start/Stop control
- `web:server:ResetFailed` → Reset failed state

## Example 2: SSH Service

### st.cmd Configuration
```bash
# Set the service name to control SSH
setServiceName("ssh.service")

# Load record instances
dbLoadRecords("db/systemd.db", "P=system:,R=ssh:,SERVICE=ssh.service")
```

### Screen Configuration
When loading `systemd_service.bob`:
- **P**: `system:`
- **R**: `ssh:`

### Resulting PVs
- `system:ssh:ServiceName` → "ssh.service"
- `system:ssh:Status` → "running", "stopped", etc.
- `system:ssh:Start` → Start/Stop control
- `system:ssh:ResetFailed` → Reset failed state

## Example 3: Custom Application Service

### st.cmd Configuration
```bash
# Set the service name to control custom app
setServiceName("myapp.service")

# Load record instances
dbLoadRecords("db/systemd.db", "P=app:,R=control:,SERVICE=myapp.service")
```

### Screen Configuration
When loading `systemd_service.bob`:
- **P**: `app:`
- **R**: `control:`

### Resulting PVs
- `app:control:ServiceName` → "myapp.service"
- `app:control:Status` → "running", "stopped", etc.
- `app:control:Start` → Start/Stop control
- `app:control:ResetFailed` → Reset failed state

## Example 4: Multiple Services

For controlling multiple services, create separate IOC instances or use different prefixes:

### Service 1: Database
```bash
setServiceName("postgresql.service")
dbLoadRecords("db/systemd.db", "P=db:,R=service:,SERVICE=postgresql.service")
```

### Service 2: Web Server
```bash
setServiceName("nginx.service")
dbLoadRecords("db/systemd.db", "P=web:,R=service:,SERVICE=nginx.service")
```

### Service 3: Message Queue
```bash
setServiceName("rabbitmq.service")
dbLoadRecords("db/systemd.db", "P=mq:,R=service:,SERVICE=rabbitmq.service")
```

## Screen Loading Tips

1. **Save Screen Configurations**: After setting up PV prefixes, save the screen with a descriptive name like `apache2_control.bob`

2. **Create Service-Specific Screens**: For production use, consider creating copies of the screen with hardcoded PV names for each service

3. **Use CSS Phoebus Workspaces**: Organize multiple service control screens in a workspace for easy access

4. **Add to CSS Phoebus Menu**: Add frequently used screens to the CSS Phoebus menu for quick access

## Troubleshooting Common Issues

### PVs Not Connecting
- Verify IOC is running: `ps aux | grep systemdIoc`
- Check PV names match exactly: `caget web:server:Status`
- Ensure service name is set correctly in st.cmd

### Buttons Not Working
- Check systemd permissions: `systemctl status apache2.service`
- Verify polkit rules are configured
- Test manual control: `systemctl start apache2.service`

### Status Not Updating
- Check IOC logs for errors
- Verify systemd service exists: `systemctl list-units | grep apache2`
- Test status PV manually: `camonitor web:server:Status`
