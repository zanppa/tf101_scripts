# tf101_scripts
Scripts for Asus EEE pad transformer tf101 running lubuntu


## DBus policy
Remember to add
```
<busconfig>
    <policy user="root">
        <allow own="org.tf101" />
    </policy>
</busconfig>
```
to `/etc/dbus-1/system.d/tf101.conf`
