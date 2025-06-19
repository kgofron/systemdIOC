# Polkit

* sudo systemctl restart polkit
* Redhat
  * sudo cp 50-serval-nopasswd.rules /etc/polkit-1/rules.d/
* Ubuntu 22.04
  * vendor provided rules
    * /usr/share/polkit-1/rules.d/
  * user defined rules
    * /etc/polkit-1/rules.d/

* ASI nictune
  * /usr/share/polkit-1/actions/com.asi.serval.policy

## Polkit debuggin
There should be no password prompt when starting serval.service

* journalctl -u polkit
* sudo systemctl restart polkit
* pkexec systemctl start serval.service
