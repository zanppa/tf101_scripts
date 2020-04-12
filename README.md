# tf101_scripts
Scripts for Asus EEE Pad Transformer TF101, runnin [DjDill's Lubuntu 14.04](https://forum.xda-developers.com/showthread.php?t=2648862) and [jrohwer's Rootbind kernel](https://forum.xda-developers.com/showthread.php?t=2347581).

## tf101monitor.py
This script should be run as root. It currently monitors the lid status and exports it over DBUS.

Copy the script to `/usr/local/bin/` and add to `/etc/rc.local`:
```
python /usr/local/bin/tf101monitor.py &
```

## fixlidclose.py
This is a modified script to shut down screen, toggel off touchpad and touchscreen (and/or keyboard). This version monitors the DBUS variables of the tf101monitor.py.

There is also a hack to work around blank screen when opening the lid.

The original script is from [DjDill's Lubuntu 14.04](https://forum.xda-developers.com/showthread.php?t=2648862).

Copy the file to `/usr/local/bin/` and add it to lxde's autostart applications list.

### DBus policy
It is necessary to create/add following to `/etc/dbus-1/system.d/tf101.conf`:

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

Otherwise the policy will either block the daemon from running or prevent the client from connecting.

### Sudoers
For the fixlidclose blank screen hack to work, `/etc/sudoers` must contain

```
# allow chvt to work around blank screen
tf101 ALL=(ALL) NOPASSWD: /bin/chvt
```
