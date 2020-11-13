# qhycmd
qhyccd command line camera utility 

## INSTALL QHYCCD SDK

Example for a version qhyccd sdk for ARMV8, please download the relevant sdk from https://www.qhyccd.com/html/prepub/log_en.html#!log_en.md

```shell

 bash> wget https://github.com/qhyccd-lzr/QHYCCD_Linux_New/blob/master/qhyccdsdk-v2.0.11-Linux-Debian-Ubuntu-armv8.tar.gz
 bash> gzip -dc qhyccdsdk-v2.0.11-Linux-Debian-Ubuntu-armv8.tar.gz | tar -xvf -
 bash> cd qhyccdsdk-v2.0.11-Linux-Debian-Ubuntu-armv8/install_scripts/
 bash> ./linux_install_qhyccd_sdk_driver.sh
``` 
 
## BUILD QHYCMD

```shell
make

```

```shell
Usage: qhycmd [-h] [-e <double>] [-cg <num>] [-co <num>] [-b <num>] [-no <num>] [-c <double>] [-wt] [-bin <str>] [-rm <num>] [-nos] [-f <str>] [-t <str>] [-fmt <str>] [-lc] [-lr] [-d] [-gui]
  -h, --help                    Show this help message
  -e, --exposure=<double>       Exposure time (seconds)
  -cg, --gain=<num>             Chip gain
  -co, --offset=<num>           Chip offset
  -b, --bpp=<num>               Bits per pixel:8, 16(default)
  -no, --expno=<num>            Number of images to record
  -c, --cool=<double>           Chip cool down temp
  -wt, --waittemp               Wait for sensor to reach temperature
  -bin, --binning=<str>         binning 1x1, 2x2, etc
  -rm, --readmode=<num>         Set camera read mode
  -nos, --nooverscan            Remove overscan area
  -f, --folder=<str>            Folder where to save images
  -t, --target=<str>            Name of the target
  -fmt, --format=<str>          Image output format (fits,tif,jpg,png)
  -lc, --listcameras            List available cameras
  -lr, --listreadmodes          List camera read modes
  -d, --debug                   Run in debug mode
  -gui, --display               Run with GUI mode

```

