# MICROSOFT FLIGHT SIMULATOR 2020 - INSTRUMENT PANEL

This is a fork of https://github.com/scott-vincent/instrument-panel with removed radio, trim and flaps knobs.

The instrument panel is running on a single monitor with plywood placed in front of it and holes cut out for each instrument. The knobs are connected to the GPIO pins of the same Raspberry Pi that is running the display.

You can also run the display on a PC (the same one running FS2020 if you wish) but you won't have the knob inputs then although you can still use the knobs in the FS2020 cockpit and the changes will immediately be reflected on the instrument panel display.

# Quick Start

Link: [Latest release of Instrument Data Link for Windows](https://github.com/scott-vincent/instrument-data-link/releases/latest/download/instrument-data-link-v2.0.0-windows-x64.zip)

Unzip instrument-data-link into its own folder and double-click instrument-data-link.exe to run it.

Unzip instrument-panel into its own folder. If you are running it on the same PC as FS2020 the default settings are correct so just double-click instrument-panel.exe to run it. Press M to switch the display to a different monitor so it's not on the same monitor as FS2020. If you then press Escape to exit the program the settings will be saved and it will run on the same monitor next time.

To run instrument-panel on a different PC, unzip instrument-panel to a folder on the new PC. Edit settings/instrument-panel.json and in the "Data Link" section change the IP address of the "Host" to the address where FS2020 is running on your local network, e.g. 192.168.0.1 - You can find the correct address of your host by running a command prompt on the host machine and running ipconfig, then scroll back and look for the first "IPv4 Address" line. Now double-click instrument-panel.exe to run it.

Note: There is a screensaver feature so the instrument panel will appear dim until you run FS2020 and select where you are flying from. It will also dim when you are flying and enter the pause menu.

If you want to use the Savage Cub panel instead of the default Cessna panel or you want to define multiple panel layouts see section "Custom Panels" below.

# Quick Start on Raspberry Pi

Follow the instructions above to download Instrument Data Link on Windows. You can then install Instrument Panel on Rapsberry Pi by doing the following:

To Install Allegro:
```
sudo apt update
sudo apt install liballegro5-dev
```
To Download:
```
tag=`curl -s -i https://github.com/scott-vincent/instrument-panel/releases/latest|grep location|cut -d '/' -f 8|tr -d '\r'`
wget https://github.com/scott-vincent/instrument-panel/releases/download/$tag/instrument-panel-$tag-raspi5-64.tar.gz
tar -xf instrument-panel-v* --one-top-level
rm instrument-panel-v*.gz
```

NOTE: Version 2.0.0 and later is built for 64-bit OS only. If you are running a 32-bit OS please download version 1.7.8 instead.

To Run:

Make sure you do this from a terminal started under a desktop session, not using SSH.
```
cd instrument-panel-v*/release/*
nano settings/instrument-panel.json    (change HOST to the IP address of your PC) 
./instrument-panel
```
# Other Panels

Autopilot Panel: https://github.com/scott-vincent/autopilot-panel

Radio Panel: https://github.com/scott-vincent/radio-panel

Power/Lights Panel: https://github.com/scott-vincent/power-lights-panel

# Releases

Ready-built releases for Windows x64 and Raspberry Pi 5 (64-bit) can be found by clicking on the Releases section on the right of this readme. The newer releases (v2.0.0 and later) will also run fine on a Raspberry Pi 4 (64-bit).

# Introduction

An instrument panel for MS FlightSim 2020. This program is designed to run on a separate computer to the one running FS2020 but you can just run it on a different monitor if you wish. It can run on either a PC or a Raspberry Pi.

You can see it running on a Raspberry Pi 4 here: https://www.youtube.com/watch?v=vA6rveEP_9o
and in action on a VFR flight with VATSIM from Gatwick to London City Airport here: https://www.youtube.com/watch?v=dC_PvOM0fI8

It requires the companion program from here: https://github.com/scott-vincent/instrument-data-link

The companion program runs on the same host as MS FS2020 and collects all the data required by the instrument panel.

The instrument panel connects to the instrument-data-link over your network and receives the data at regular intervals so that the instruments can be drawn with up-to-date values.

You can run as many instrument panels as you like. They all connect to the same data-link. Each panel must be configured to point to the host where data-link is running. You do this by editing the settings file and updating the Data Link Host IP address. If you just want to test the instrument panel on the same host where FS2020 is running leave the IP address as 127.0.0.1 and it will connect to the local host.

Blaine Leckett made his own instrument panel using my software and he has a very good build video here: https://www.youtube.com/watch?v=iHpFrYcHiJM

This program was heavily inspired by Dave Ault and contains original artwork by him.

  http://www.learjet45chimera.co.uk/
  
  https://hangar45.net/hangar-45-forum/topic/standby-gauge-software-by-dave-ault
  
It has been completely rewritten and updated to use Allegro5.

To install Allegro (5.2.7) on Windows:

  Load instrument-panel.sln into Visual Studio 2022 Community Edition.
  When you build the solution it will automatically download Allegro
  as it is specified in packages.config. A new folder called
  packages will be created under the main folder.

To install Allegro on Raspberry Pi:
```
  sudo apt install liballegro5-dev
```
### KEYS
```
p ........ Adjust position and size of individual instruments.
v ........ Adjust FlightSim variables. Simulates changes even if no FlightSim connected.
m ........ Move the display to the next monitor if multiple monitors are connected.
s ........ Enable/disable shadows on instruments. Shadows give a more realistic 3D look.
Esc ...... Quit the program.
```
To make adjustments use the arrow keys. Up/down arrows select the previous or next setting and left/right arrows change the value. You can also use numpad left/right arrows to make larger adjustments.

You can choose which instruments are included in the panel by setting the Enabled attribute for each instrument in the settings file:
```
  settings/instrument-panel.json
```
On Raspberry Pi you can configure hardware Rotary Encoders for each instrument. Each rotary encoder is connected to two BCM GPIO pins (+ ground on centre pin). See individual instruments for pins used. Not all instruments have manual controls.

# Custom Panels

Instrument Panel now supports multiple panels in case you decide to build a Savage Cub panel for example instead of a Cessna panel. You need to create a settings file for each custom panel and you will find a Savage Cub one already in the settings folder.

To use it, on Windows you need to right click on instrument-panel.exe and create a shortcut to it, then add the settings file name to the Target, e.g.

Target: C:\instrument-panel\instrument-panel.exe savage-cub.json
  
On Raspberry Pi just supply the settings file name on the command line when you call instrument-panel.exe.

If no settings file is supplied, "instrument-panel.json" will be used and the default Cessna panel will be displayed. 

# Intelligent Instruments

Some of the instruments are 'intelligent' and will adapt to whatever aircraft you are currently flying. For example, adding ASI to your panel will allow it to switch to the Savage Cub version when you are flying that particular aircraft and ALT will switch to a version with a digital readout when you are flying a fast aircraft, e.g. one of the airliners.

Use a specific instrument in your panel if you don't want it to auto switch, for example, use "ASI Savage Cub" instead of "ASI" to always display the Savage Cub version of the instrument.

Note that only a few specific instruments have been created so far as this project is a work in progress.  
