# Formatting a USB Drive for Linux UEFI Validation Project  
### [LUV Project](https://01.org/linux-uefi-validation) 

The following steps explain how to prepare a USB drive for the Linux UEFI 
Validation (LUV) Project. While some operating system tools require formatting 
or pre-formatting, the LUV Project does not. There is no need to pre-format 
your USB drive. But do be aware that **the operations in this tutorial will erase
all data from the USB drive**; be sure any important data on the drive is 
backed up elsewhere as needed.  

## Format the USB
Command-line directions are as follows:  

1. Check this link for the latest version and modify as needed.  
	``` 
	https://01.org/linux-uefi-validation/downloads
	```


2. Use the `wget` command to download the compressed file:
	```
	$ wget https://download.01.org/linux-uefi-validation/v1.2/luv-live-v1.2-rc1.tar.bz2
	```


3. Extract the folder and image from the file:
	```
	$ tar -xjvf luv-live-v1.2-rc1.tar.bz2`
	```


4. Change to the directory and look at the newly-extracted contents; there should be a file ending in `.img` or `.iso`:
	```
    $ cd luv-live-v1.2-rc1/
    $ ll
    total 77140
	drwxr-xr-x 2 lsc lsc     4096 Nov  9 17:34 .
	drwxr-xr-x 3 lsc lsc     4096 Nov 18 14:13 ..
	-rw-r--r-- 1 lsc lsc 78955008 Nov  9 17:34 luv-live-v1.2-rc1.img
	-rw-r--r-- 1 lsc lsc      765 Nov  9 17:33 README
	```


5. Insert the USB drive (for the following example, we're using a 4GB USB stick) and confirm the block ID letter of the USB disk with the `lsblk` command. Here the USB shows up under `/media`, as the following example shows:
	```
	$ lsblk
  	NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
  	sda      8:0    0 223.6G  0 disk 
	─sda1   8:1    0   118G  0 part 
	.
	.
	sdc      8:32   1   3.7G  0 disk 
	└─sdc1   8:33   1   3.6G  0 part /media/lsc/UUI
	sr0     11:0    1  1024M  0 rom
  	```

	In the above example, the USB drive is detected at `/media` and is called `sdc`. Be very careful to correctly identify this. The sdc is less than 4GB, so we can be pretty sure we've identified the location;  and confirm that it's not mounted:
	```
	$ umount /dev/sdc
		umount: /dev/sdc is not mounted (according to mtab)
	```	

		
6. Use the `dd` command to convert and copy; specify block size, if necessary:
```
$ sudo dd bs=4M if=/some/local/luv-live-v1.2-rc1/luv-live-v1.2-rc1.img of=/dev/sdc && sync
[sudo] password: 
18+1 records in
18+1 records out
78955008 bytes (79 MB) copied, 17.8973 s, 4.4 MB/s
*for more info on the dd command parameters please see https://en.wikipedia.org/wiki/Dd_(Unix)
```


7. The USB drive should now be ready to go!  If you insert the drive on a live operating system, there will be two partitions named `'boot'` and `'luv-results'` respectively:
	* `boot/` -- contains all the files needed to boot the system
	* `luv-results/` -- contains two directories: 
	1. `luv_results/parsed` -- contains the results (pass/fail) of the live tests
	2. `luv_results/raw` -- contains data useful for debugging


## Miscellaneous Info 
UEFI is Unified Extensible Firmware Interface, a BIOS-like bootloader environment with advanced platform-agnostic capabilities for testing and diagnostic reporting.


## Features 
1. Streamlines the booting process.
	
2. Contains an extensible pre-OS / “preload” environment with a suite of testing tools designed to identify firmware implementation 
	issues.
	
3. Can test in areas not previously available, such as the 	interactions among the bootloader, Linux kernel and firmware itself.

4. Can test firmware with more realistic real-world scenarios (under loads such as as the firmware will likely be used in the real world).


## Links
Source / [Github Repo](https://github.com/01org/luv-yocto)  
[Mailing List](mailto:luv@lists.01.org)   
