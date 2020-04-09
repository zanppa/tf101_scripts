# tf101_scripts
Scripts for Asus EEE pad transformer tf101 running lubuntu

## tf101monitor.py
This script should be run as root. It monitors the lid status and exports it over DBUs.

### DBus policy
Remember to add
```
<busconfig>
    <policy user="root">
        <allow own="org.tf101" />
    </policy>
    <policy group="tf101">
        <allow send_destination="org.tf101" />
    </policy>
</busconfig>
```
to `/etc/dbus-1/system.d/tf101.conf`
