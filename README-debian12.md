# MPSS 3.8.6 host driver on Debian 12 (Linux 6.1)

Host kernel module (`mic.ko`) for Intel Xeon Phi x100 (Knights Corner:
3120/5110P/5120D/7120) ported to Linux 6.1. See the top commit message for
the full list of API changes and bug fixes.

**Status:** compile-tested against linux-6.1.193. Not yet tested on hardware.

## Build and install

```
sudo apt install build-essential linux-headers-$(uname -r) dkms
make MIC_CARD_ARCH=k1om
sudo make install sysconfdir=/etc
sudo depmod -a
```

`sysconfdir=/etc` matters: the default puts modprobe/udev files under
`/usr/local/etc`, which Debian does not read.

### With DKMS (rebuilds automatically on kernel updates)

```
sudo cp -r . /usr/src/mpss-modules-3.8.6-linux6
sudo dkms install mpss-modules/3.8.6-linux6
sudo install -m644 mic.conf /etc/modprobe.d/mic.conf
sudo install -m644 udev-mic.rules /etc/udev/rules.d/50-udev-mic.rules
```

## First load

```
lspci -nn | grep -i 8086:225          # card visible?
sudo modprobe mic
dmesg | tail -40                      # look for "mic_probe ... as board #0"
ls -l /dev/mic/                       # expect ctrl and scif
cat /sys/class/mic/mic0/state         # expect "ready"
```

Card boot still needs the MPSS 3.8.6 userspace (mpssd, micctrl, libscif,
card uOS image under /usr/share/mpss/boot) from the RHEL7 RPMs.
