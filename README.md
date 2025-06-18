# systemdIOC
Start systemd serval.service from EPICS
1. make; ./systemd_control
2. build_andrun.sh
3. See ASI PolicyKit for netTune: com.asi.serval.policy

## Service not starting
```
* Failed to start Serval.
* serval.service: Start request repeated too quickly.
* serval.service: Failed with result 'exit-code'.
```
* systemctl reset-failed serval.service

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
    * sudo nano /etc/polkit-1/rules.d/50-serval-nopasswd.rules
```
polkit.addRule(function(action, subject) {
    if (action.id == "org.freedesktop.systemd1.manage-units" &&
        action.lookup("unit") == "serval.service") {
        if (subject.user == "your_username") {
            return polkit.Result.YES;
        }
    }
});
```
2. sudo visudo -f /etc/sudoers.d/serval-control
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
* sudo systemctl reset-failed serval.service
* ps aux | grep systemdIoc | cat
* busctl call org.freedesktop.systemd1 /org/freedesktop/systemd1 org.freedesktop.systemd1.Manager GetUnit serval.service | cat
* busctl call org.freedesktop.systemd1 /org/freedesktop/systemd1 org.freedesktop.systemd1.Manager GetUnitByPID 1 | cat
