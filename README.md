# systemdIOC
Start systemd serval.service from EPICS

## Service not starting
* Failed to start Serval.
* serval.service: Start request repeated too quickly.
* serval.service: Failed with result 'exit-code'.

## Reset
* systemctl reset-failed <service name>

## Info
* man systemd.directives 
	* {SystemdFailedUnit, StartUnit, StopUnit, ...}
* man systemd.service

## no password
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
* sudo visudo -f /etc/sudoers.d/serval-control
```
your_username ALL=(ALL) NOPASSWD: /path/to/your/systemd_control
```
* Permissions
```
sudo chown root:root /path/to/your/systemd_control
sudo chmod 755 /path/to/your/systemd_control
```
* Run without password
```
sudo ./systemd_control
```

