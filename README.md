# mpradio
Morrolinux's Pirate radio (PiFmRDS implementation with Bluetooth and mp3 support) for all Raspberry Pi models

Exclusively tested on Minimal Raspbian (ARM)

# Features
- [x] Safely shutdown your Pi by unplugging the USB stick
- [x] Persistent playlists (remember the playlist/playback status across reboots)
- [x] Resume track from its playback status hh:mm:ss
- [x] Shuffle on/off
- [x] Scrolling RDS to overcome 8-chars limitation 
- [x] Skip to the next song by pressing a push-button (GPIO-connected on pin 18)
- [x] Safely shutdown by holding the push-button (GPIO-connected on pin 18)
- [x] Stream audio over FM or 3.5mm Jack (Bluetooth speaker via jack audio output)
- [x] Send mp3 files to the Pi via Bluetooth
- [x] Bluetooth OTA file management on the Pi with applications such as "Bluetooth Explorer Lite"
- [ ] Read metadata from the mp3 files 
- [ ] Display Android notifications over RDS?
- [ ] Bluetooth companion app (for android) 
- [ ] Automatically partition the sdcard for a dedicated mp3 storage space (instead of using a USB drive)

# Known issues
- Due to a design flaw in BCM43438 WIFI/BT chipset, you might need to disable WiFi if you experience BT audio stuttering on Pi Zero W and Pi 3: https://github.com/raspberrypi/linux/issues/1402
- Boot can take as long as 1:30 min on the Pi1 and 2 due to BT UART interface missing on the board. `sudo systemctl mask dev-serial1.device` should help 

# Installation

First make sure your Raspbian is up to date:

` sudo apt-get update && sudo apt-get -y full-upgrade && sudo apt-get install git`

` git clone https://github.com/morrolinux/mpradio.git mpradio-master `

` cd mpradio-master/install && sudo ./install.sh`

# Configuration
By default, mpradio will always be running automatically after boot once installed. No additional configuration is needed.
However, you can change the FM streaming frequency (which is otherwise defaulted to 107.0) by placing a file named pirateradio.config in the root of a USB key (which of course, will need to stay plugged for the settings to be permanent)

pirateradio.config example:
```
[PIRATERADIO]
frequency=107.0
btGain=1.7            	;gain setting for bluetooth streaming
storageGain=1.3       	;gain setting for stored files streaming
output=fm		;[analog/fm] to stream thru FM or 3.5mm jack 
btBoost=true		;Enhance Bluetooth audio. This might add a little latency

[PLAYLIST]
persistentPlaylist=true
resumePlayback=true   	;require persistentPlaylist to be enabled 
shuffle=true 

[RDS]
updateInterval=3      	;seconds between RDS refresh. lower values could result in RDS being ignored by your radio receiver
charsJump=6           	;how many characters should shift between updates [1-8]

```
# Usage
It (should) work out of the box. You need your mp3 files to be on a FAT32 usb stick. (along with the config file if you need to override default settings)
You can **safely** shut down the Pi by unplugging the stick and waiting for about 5 seconds until the status led stops blinking.
If you enabled "persistentPlaylist" option, your Pi will never play the same song twice before consuming the full playlist.
If you add new songs on the usb stick, with "persistentPlaylist" enabled they won't be played until the current playlist is consumed. You can "rebuild" the playlist (looking for new recently added files) if needed:
- boot your Pi with usb stick unplugged. (current playlist will be erased)
- plug in your usb stick and unplug it again (this will cause a shut down) 
- power on your Pi once again, with the usb key in it.
- You're done! (mpradio will rebuild the playlist, counting the new files as well)

Also, please remember that (although that would be probably illegal) you can test FM broadcasting by plugging a 20cm wire on the **GPIO 4** of your Pi

# Update 
` rm -rf mpradio-master/ ` 
and then:

installation steps

OR, if you are a git guy:

`cd mpradio-master && git fetch origin && git reset --hard origin/master && cd install && sudo ./install.sh`

# Uninstallation / Removal
` cd mpradio-master/install && sudo ./install.sh remove `

# Debugging / Troubleshooting
mpradio is launched as a service (via systemd) after boot completed

check whether the service is running or not: 

` $ sudo systemctl status mpradio `

start or stop the service:

` $ sudo systemctl [start/stop] mpradio `

As for Bluetooth:

Bluetooth connection logs are under ` /var/log/bluetooth_dev `

if Raspberry's Bluetooth is not showing up, check if the interface is UP and the bt-setup script has been executed:

` $ hciconfig `

` $ sudo systemctl status bt-setup `

if you are having issues with bluetooth audio pairing, please also check if simple-agent service is running:

` $ sudo systemctl status simple-agent `

if you are having issues with bluetooth not connecting once it's paired, please check weather bluealsa is running or not:

` $ sudo systemctl status bluealsa `


A simple schematic of how things work together:

![Alt text](/doc/mpradio_schematic.png?raw=true "mpradio schematic")

# Warning and Disclaimer
mpradio relays on PiFmRds for FM-Streaming feature. Please note that in most states, transmitting radio waves without a state-issued licence specific to the transmission modalities (frequency, power, bandwidth, etc.) is illegal. Always use a shield between your radio receiver and the Raspberry. Never use an antenna. See PiFmRds Waring and Disclamer for more informations.
