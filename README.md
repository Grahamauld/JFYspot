# jfyspot
This project was started because I have a SMA inverter and use SBFspot to pump data into 123solar ( https://www.123solar.org/ ).
I installed additional panels and a JFY JSI-3000 inverter ( http://www.jfy-tech.com/ ) and struggled to find software to extract data nevermind pop it into 123solar...

I found solarmonj ( https://github.com/jcroucher/solarmonj ) to be the nearest thing to what I was after so after some cut'n'shut work, here we are.

Yes it runs on linux, no it dosn't run on windows, yes it runs on Raspberry Pi.

### Requirements

To run this project you will need:

* JFY JSI Series Inverter
* Serial cable (link pins 4&9 at the inverter end - cable only needs pins 2,3,5 connected 1:1)
* Probably some form of RS232 to USB converter
* Some flavour of Linux

### Building

In order to build this project you will need:

* make
* g++

### Setup

* ./configure
* make
* make install
* jfyspot --help
* probably install and configure 123solar...

