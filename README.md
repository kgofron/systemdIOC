# systemdIOC
Control any systemd service from EPICS

This EPICS IOC provides generic systemd service control capabilities, allowing you to start, stop, reset failed state, and monitor the status of any systemd service.

## Quick Start
1. Configure the service name in `iocBoot/iocsystemd/st.cmd`
2. Set up permissions (see Permissions section below)
3. Build and run: `make; ./systemd_control`
4. Or use the build script: `build_andrun.sh`

## Configuration

### Setting the Service Name
Edit `iocBoot/iocsystemd/st.cmd` and modify these lines:
```bash
# Set the service name to control (default: serval.service)
setServiceName("your-service.service")

# Load record instances with service-specific descriptions
dbLoadRecords("db/systemd.db", "P=yourprefix:,R=service:,SERVICE=your-service.service")
```

### Example Configurations
- For SSH service: `setServiceName("ssh.service")`
- For Apache web server: `setServiceName("apache2.service")`
- For custom service: `setServiceName("myapp.service")`

## Service not starting
```
* Failed to start YourService.
* your-service.service: Start request repeated too quickly.
* your-service.service: Failed with result 'exit-code'.
```
* systemctl reset-failed your-service.service

## Info
* man systemd.directives 
	* {SystemdFailedUnit, StartUnit, StopUnit, ...}
* man systemd.service

## Permissions

### sudoers
```
Sudoers approach: More flexible, easier to revoke access
```

1. Polkit rule file
    * sudo nano /etc/polkit-1/rules.d/50-systemd-nopasswd.rules
```
polkit.addRule(function(action, subject) {
    // Allow specific users to control systemd services without password
    var allowedUsers = ["your_username", "epics", "ioc"];
    if (allowedUsers.indexOf(subject.user) !== -1) {
        if (action.id.indexOf("org.freedesktop.systemd1") === 0) {
            return polkit.Result.YES;
        }
    }
    
    // Default: require authentication for other actions
    return polkit.Result.AUTH_SELF_KEEP;
});
```
2. sudo visudo -f /etc/sudoers.d/systemd-control
```
your_username ALL=(ALL) NOPASSWD: /path/to/your/systemd_control
```
3. Permissions
```
sudo chown root:root /path/to/your/systemd_control
sudo chmod 755 /path/to/your/systemd_control
```
* Run without password
    * sudo ./systemd_control

### setuid root
```
The setuid approach:
Doesn't require sudo
Doesn't require entering a password
Is more secure because it drops privileges after getting the bus connection
Still requires the polkit rule to allow the specific user to control the service {not on ubuntu 22.04??}
Setuid approach: More convenient, slightly more secure, but harder to revoke access
```
1. Drop privileges after getting the bus connection
{unistd.h, setuid(getuid()}
```
#include <systemd/sd-bus.h>
#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>

class SystemdController {
private:
    sd_bus* bus = nullptr;

public:
    SystemdController() {
        // Using system bus to control system-wide services
        // Note: Access control should be managed through polkit rules
        int ret = sd_bus_default_system(&bus);
        if (ret < 0) {
            throw std::runtime_error("Failed to connect to system bus: " + std::string(strerror(-ret)));
        }

        // Drop privileges after getting the bus connection
        if (setuid(getuid()) != 0) {
            throw std::runtime_error("Failed to drop privileges");
        }
    }

    ~SystemdController() {
```
2. setuid root
```
sudo chown root:root systemd_control
sudo chmod u+s systemd_control
```

* Run without password, and without sudo
    * ./systemd_control

## Commands
* sudo systemctl reset-failed your-service.service
* ps aux | grep systemdIoc | cat
* busctl call org.freedesktop.systemd1 /org/freedesktop/systemd1 org.freedesktop.systemd1.Manager GetUnit your-service.service | cat
* busctl call org.freedesktop.systemd1 /org/freedesktop/systemd1 org.freedesktop.systemd1.Manager GetUnitByPID 1 | cat
* watch -n 2 systemctl status your-service.service

## EPICS Records

The IOC provides three EPICS records for each service:

1. **Start/Stop Record** (`$(P)$(R)Start`): Binary output record to start (1) or stop (0) the service
2. **Reset Failed Record** (`$(P)$(R)ResetFailed`): Binary output record to reset the failed state of the service
3. **Status Record** (`$(P)$(R)Status`): String input record showing the current service status (running, stopped, starting, stopping, etc.)

## Device Support

The IOC uses generic device support types:
- `Systemd`: For start/stop and status operations
- `SystemdReset`: For reset failed operations

These replace the previous serval-specific device types.
