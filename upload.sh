#!/bin/bash
/home/ubuntu/sysroot/usr/bin/aarch64-buildroot-linux-gnu-strip -s emulationstation
7zr a emulationstation.7z emulationstation
sftp root@192.168.1.7 <<END
put emulationstation.7z
END
rm emulationstation.7z
rm emulationstation
