# tf101_scripts
Scripts for Asus EEE Pad Transformer TF101, running [DjDill's Lubuntu 14.04](https://forum.xda-developers.com/showthread.php?t=2648862) and [jrohwer's Rootbind kernel](https://forum.xda-developers.com/showthread.php?t=2347581).

## tf101monitor.py
This script should be run as root. It currently monitors the lid status and exports it over DBUS.

Copy the script to `/usr/local/bin/` and add to `/etc/rc.local`:
```
python /usr/local/bin/tf101monitor.py &
```

## fixlidclose.py
This is a modified script to shut down screen, toggle off touchpad and touchscreen (and/or keyboard). This version monitors the DBUS variables of the tf101monitor.py.

There is also a hack to work around blank screen when opening the lid. This was only necessary with newer Xorg which used Nouveau drivers (because the Tegra driver was no longer supported), and the feature is now removed.

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

## dpms_detector.py
This script is not needed if Tegra drivers are used (instead of Nouveau), i.e. old enough Xorg.

This script is intended to run on background and run a command when the system returns from DPMS suspend state, i.e. from blank screen. There is some bug (in X?) which leaves the screen blank even though the backlight comes on, and only changing virtual terminal brings the picture back. This automates the task, and allows returning from DPMS suspend even without keyboard.

Script should be run with
```
python dpms_detector.py --display ":0" --command_on "sudo chvt 1 && sudo chvt 7"
```
sudoers should be modified like above.


## rotate_screen.sh
This script can be used to rotate screen & touch screen & other mice.

In my set-up this is bind to ctrl+shift+arrows.
