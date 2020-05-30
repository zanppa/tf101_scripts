#!/usr/bin/python

import dbus, gobject, sys, os
from dbus.mainloop.glib import DBusGMainLoop
from subprocess import Popen, PIPE

p1 = Popen(['xinput'], stdout=PIPE)
p2 = Popen(['grep', 'asusec'], stdin=p1.stdout, stdout=PIPE)
p3 = Popen(['awk', '{print$3}'], stdin=p2.stdout, stdout=PIPE)
p4 = Popen(['cut', '-d=', '-f2'], stdin=p3.stdout, stdout=PIPE)
KBDEV = p4.communicate()[0].strip()

p1 = Popen(['xinput'], stdout=PIPE)
p2 = Popen(['grep', 'elantech'], stdin=p1.stdout, stdout=PIPE)
p3 = Popen(['awk', '{print$4}'], stdin=p2.stdout, stdout=PIPE)
p4 = Popen(['cut', '-d=', '-f2'], stdin=p3.stdout, stdout=PIPE)
SCRDEV = p4.communicate()[0].strip()

del p1, p2, p3, p4

pow_prop_iface = None
pow_iface = None

def handle_lidclose(*args):
    closed = pow_prop_iface.Get('',
                                'LidIsClosed')
    if closed:
        print "lid is closed, disabling the touchscreen"
#        os.system('xdg-screensaver lock')
        os.system('xset dpms force off')
        os.system('xinput set-int-prop '+KBDEV+' "Device Enabled" 8 0')
        os.system('xinput set-int-prop '+SCRDEV+' "Device Enabled" 8 0')
    else:
        print "lid is open, enabling the touchscreen"
        os.system('xinput set-int-prop '+KBDEV+' "Device Enabled" 8 1')
        os.system('xinput set-int-prop '+SCRDEV+' "Device Enabled" 8 1')
        os.system('xset dpms force on')
        # os.system('sudo chvt 1 && sudo chvt 7') # This is now handled by other scipt

def main():
    global pow_prop_iface, pow_iface

    DBusGMainLoop(set_as_default=True)

    bus = dbus.SystemBus()

    power_proxy = bus.get_object('org.tf101',
                                '/org/tf101')

    pow_prop_iface = dbus.Interface(power_proxy,
                                    'org.freedesktop.DBus.Properties')
    pow_iface = dbus.Interface(power_proxy,
                               'org.tf101')

    print "Registering a signal receiver for tf101 events..."

    bus.add_signal_receiver(handle_lidclose,
                            dbus_interface="org.tf101",
                            signal_name="LidChanged")


    loop = gobject.MainLoop()
    loop.run()

if __name__ == '__main__':
    main()
