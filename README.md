# simplefs

Demonstrative fs that's a reduction of the framework shown in semfs by Nemo. 

Simplefs accepts writes to the ctl file, logs the message, then prints out its history in the log file.

## Build

	mk

## Install

	mk install

## Usage

An example session:

	tenshi% mk
	6c -FTVw main.c
	6l  -o 6.out main.6
	tenshi% ./6.out
	tenshi% echo hi > /mnt/simplefs/log
	echo: write error: only ctl may be written to
	tenshi% echo hi > /mnt/simplefs/ctl
	tenshi% cat /mnt/simplefs/log
	hi
	tenshi% echo ducks > /mnt/simplefs/ctl
	tenshi% cat /mnt/simplefs/log
	hi
	ducks
	tenshi%


## References

- [9p(5)](http://man.postnix.us/9front/5/intro)
- [semfs](https://bitbucket.org/henesy/9intro/src/default/ch13/semfs/)
- [ctlfs](http://contrib.9front.org/mischief/sys/src/cmd/proc/src/core/ctlfs.c)
- [lib9p/ramfs](http://mirror.postnix.us/plan9front/sys/src/lib9p/ramfs.c)
