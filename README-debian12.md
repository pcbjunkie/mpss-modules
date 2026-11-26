# MPSS 3.8.6 host driver on Debian 12 (Linux 6.1)

Host kernel module (`mic.ko`) for Intel Xeon Phi x100 (Knights Corner:
3120/5110P/5120D/7120) ported to Linux 6.1. See the port commit message for
the full list of API changes and bug fixes.

**Status:** hardware-tested on a Xeon Phi 5110P in a Dell R720, Debian 12,
kernels 6.1.0-50 and 6.1.0-56. Load, card boot (online in ~22 s), SCIF,
virtual network, `ssh mic0`, `micsmc`, and auto-start at host boot via
DKMS and `mpss.service` all work.

## Build and install (DKMS, rebuilds on kernel updates)

```
sudo apt install build-essential linux-headers-$(uname -r) dkms
sudo mkdir /usr/src/mpss-modules-3.8.6-linux6
git archive HEAD | sudo tar -x -C /usr/src/mpss-modules-3.8.6-linux6
sudo dkms install mpss-modules/3.8.6-linux6
```

Note: on Debian, `dkms` pulls in `linux-headers-amd64`, which can also
upgrade `linux-image-amd64` to a newer 6.1.0-N kernel. DKMS builds for it.

Do **not** install the bundled `udev-mic.rules` (RHEL-only; uses the removed
udev `NAME=` key) or `mic.conf` (sets `reg_cache=1`, which differs from the
driver default and is untested here). For group access to the device nodes:

```
# /etc/udev/rules.d/50-mic.rules
KERNEL=="scif", SUBSYSTEM=="mic", GROUP="mic", MODE="0660"
KERNEL=="ctrl", SUBSYSTEM=="mic", GROUP="mic", MODE="0660"
```

Without DKMS: `make MIC_CARD_ARCH=k1om`, then `sudo insmod ./mic.ko`.

## First load

```
lspci -nn | grep -i 8086:225          # card visible?
sudo modprobe mic
dmesg | tail -40                      # look for "mic_probe ... as board #0"
ls -l /dev/mic/                       # expect ctrl and scif
cat /sys/class/mic/mic0/state         # expect "ready"
```

Card boot needs the MPSS 3.8.6 userspace (mpssd, micctrl, libscif, card uOS
image under /usr/share/mpss/boot) from Intel's RHEL7 RPMs, then
`micctrl --initdefaults` and `systemctl start mpss`.

## Stuck shutdowns: `shutdown_timeout`

The card's uOS often stops partway through an orderly shutdown: it sends its
SCIF exit but never signals the host that it halted (4 of 6 shutdowns on the
test card). The original driver then left the card in `shutdown` forever. The
SCIF watchdog stops in that state, and mpssd's `ShutdownTimeout` only applies
to `micctrl --wait` and `mpss stop`. A plain `micctrl --reboot` needed a
manual `micctrl --reset`.

This port adds a host-side timeout: if a card is still in `shutdown` after
`shutdown_timeout` seconds (default 120, `0` = wait forever), the driver
logs `mic0: shutdown did not complete in N s, resetting` and resets it to
`ready`. A pending `--reboot` then boots it again. Normal shutdowns finish in
~20 s and are unaffected. Change it at runtime with
`/sys/module/mic/parameters/shutdown_timeout`, or with
`options mic shutdown_timeout=N` in `/etc/modprobe.d/`.

Keep systemd's stop timeout for `mpss.service` (300 s in Intel's unit) longer
than `shutdown_timeout`.

Known gap: a `poweroff` run *on the card* does not reach the host as a
shutdown. The host keeps showing `online`, and the SCIF watchdog does not
notice either. Use `micctrl --shutdown` or `--reboot` from the host instead.
