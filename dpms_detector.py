#!/usr/bin/env python

# dpms_detector.py -- A tool to detect DPMS status and launch a 
# command when mode is returned from suspend/standby to on
#
# Original code from https://github.com/BlueSkyDetector/dpms_detector
#
#
# MIT License
#
# Copyright (c) 2017 Takanori Suzuki
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import ctypes
import struct
from logging import getLogger, StreamHandler, Formatter, WARN, INFO, DEBUG
import os

logger = getLogger(__name__)
formatter = Formatter('%(asctime)s - %(levelname)s: %(message)s',
                      '%Y/%m/%d %H:%M:%S')
handler = StreamHandler()
handler.setFormatter(formatter)
handler.setLevel(INFO)
logger.setLevel(INFO)
logger.addHandler(handler)
logger.propagate = False

ctypes.cdll.LoadLibrary('libXext.so')
libXext = ctypes.CDLL('libXext.so')

DPMSNONE = -2
DPMSFAIL = -1
DPMSModeOn = 0
DPMSModeStandby = 1
DPMSModeSuspend = 2
DPMSModeOff = 3


def get_DPMS_state(display_name_in_byte_string=b':0'):
    state = DPMSFAIL
    if not isinstance(display_name_in_byte_string, bytes):
        raise TypeError
    display_name = ctypes.c_char_p()
    display_name.value = display_name_in_byte_string
    libXext.XOpenDisplay.restype = ctypes.c_void_p
    display = ctypes.c_void_p(libXext.XOpenDisplay(display_name))
    dummy1_i_p = ctypes.create_string_buffer(8)
    dummy2_i_p = ctypes.create_string_buffer(8)
    if display.value:
        if libXext.DPMSQueryExtension(display, dummy1_i_p, dummy2_i_p)\
           and libXext.DPMSCapable(display):
            onoff_p = ctypes.create_string_buffer(1)
            state_p = ctypes.create_string_buffer(2)
            if libXext.DPMSInfo(display, state_p, onoff_p):
                onoff = struct.unpack('B', onoff_p.raw)[0]
                if onoff:
                    state = struct.unpack('H', state_p.raw)[0]
        libXext.XCloseDisplay(display)
    return state


def main():
    import time
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--display',
                        action='store',
                        required=True,
                        help='Specify display name in format \':0.0\'')
    parser.add_argument('--command_on',
                        action='store',
                        required=False,
                        default=None,
                        help='Specify command to run when returning from suspend')
    args = parser.parse_args()
    logger.info('\'%s\' started' % __file__)
    display_name_in_byte_string = args.display.encode('ascii')
    cmd_on = args.command_on
    dpms_state = DPMSNONE
    while True:
        new_dpms_state = get_DPMS_state(display_name_in_byte_string)
        if dpms_state == DPMSNONE:
            dpms_state = new_dpms_state
        elif dpms_state != new_dpms_state:
            dpms_state = new_dpms_state
            if dpms_state == DPMSFAIL:
                logger.info('DPMS state is detected as [DPMSFAIL]')
            elif dpms_state == DPMSModeOn:
                logger.info('DPMS state is detected as [DPMSModeOn]')
                if cmd_on:
                    os.system(cmd_on)
            elif dpms_state == DPMSModeStandby:
                logger.info('DPMS state is detected as [DPMSModeStandby]')
            elif dpms_state == DPMSModeSuspend:
                logger.info('DPMS state is detected as [DPMSModeSuspend]')
            elif dpms_state == DPMSModeOff:
                logger.info('DPMS state is detected as [DPMSModeOff]')
        time.sleep(1)


if __name__ == '__main__':
    main()
