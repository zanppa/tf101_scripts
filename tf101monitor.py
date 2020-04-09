#!/usr/bin/python
# Monitor lid status and export it it DBUS

import dbus, dbus.service
from dbus.mainloop.glib import DBusGMainLoop
import gobject
import subprocess

event = '/dev/input/by-path/platform-gpio-keys.0-event'

class Tf101Server(dbus.service.Object):
    def __init__(self):
        self.bus = dbus.service.BusName('org.tf101', bus=dbus.SystemBus())
        dbus.service.Object.__init__(self, self.bus, '/org/tf101')
        self.lid_status = 0

    def SetLid(self, status):
        if self.lid_status != status:
            LidChanged(status)

    # Emit a signal whenever the lid status changes
    @dbus.service.signal('org.tf101', signature='q')
    def LidChanged(self, status):
        self.lid_status = status

    # Property handlers
    @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ss', out_signature='q')
    def Get(self, iface_name, prop_name):
        if iface_name == 'org.tf101':
            if prop_name == 'LidisClosed':
                return self.lid_status
        else:
            raise dbus.exceptions.DBusException('interface not implemented')

    @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ssq')
    def Set(self, iface_name, prop_name, value):
        pass


tf101 = None

def get_lid_status():
    global tf101

    try:
        # Fetch the lid status from event
        retcode = subprocess.call(['evtest', '--query', event, 'EV_SW', 'SW_LID'])

        print('{:d}, {}'.format(retcode, retcode == 10))

        # Return code is 0 for closed lid, 10 for open
        if retcode == 0:
            tf101.SetLid(0)
        elif retcode == 10:
            tf101.SetLid(1)
        else:
            print('Error in reading lid status: unknown return code')

    except:
        print('Error in reading lid status')

    return True


if __name__ == '__main__':
    DBusGMainLoop(set_as_default=True)

    tf101 = Tf101Server()

    # Monitor lid status
    gobject.timeout_add(1000, get_lid_status)

    loop = gobject.MainLoop()
    loop.run()
