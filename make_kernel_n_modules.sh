#!/bin/bash

arch=arm
cross_compile=arm-linux-gnueabi-
nr_cores=3
dest_dir="/home/celemine1gig/kirkwood/ls-wvl/kernels/3.16-1"
tftp_dir="/tftpboot"
make_cmd="make ARCH=$arch CROSS_COMPILE=$cross_compile -j$nr_cores"
dtb_file="kirkwood-lswvl"

if [ ! -z $1  ] && [ "$1" = "clean" ]
then
	echo -e "Cleaning the kernel source directory now.\n---------------------\n"
	$make_cmd clean
	sleep 3
fi 

$make_cmd zImage 2>zImage_errors.txt
if [ "$?" = "0" ]
then
	echo -e "'zImage' compiled fine.\n---------------------\n"
	sleep 3	
else
	echo "ERRORS occured while compiling 'zImage'. Exiting now!"
	exit 1
fi

$make_cmd modules 2>modules_errors.txt
if [ "$?" = "0" ]
then
	echo -e "'modules' compiled fine.\n---------------------\n"
	sleep 3
else
	echo "ERRORS occured while compiling 'modules'. Exiting now!"
	exit 2
fi

$make_cmd $dtb_file.dtb 2>dtb_errors.txt
if [ "$?" = "0" ]
then
	echo -e "'dtb' compiled fine.\n---------------------\n"
	sleep 3	
else
	echo "ERRORS occured while compiling 'dtb'. Exiting now!"
	exit 3
fi

if [ -f arch/arm/boot/dts/$dtb_file.dtb ] && [ -f arch/arm/boot/zImage ]
then
	echo -e "Creating the appended zImage + dtb file.\n---------------------\n"
	cat arch/arm/boot/zImage arch/arm/boot/dts/$dtb_file.dtb > arch/arm/boot/zImage.fdt
	sleep 3
else
	echo "ERROR! Either dtb-file or zImage not found."
	exit 4
fi

if [ -f arch/arm/boot/zImage.fdt ]
then
	echo -e "Creating the u-Boot image.\n---------------------\n"
	mkimage -A arm -O linux -T kernel -C none -a 0x00008000 -e 0x00008000 -n "3.16-1-kirkwood-ls-wvl-1.0" -d arch/arm/boot/zImage.fdt arch/arm/boot/uImage
	sleep 3
else
	echo "ERROR! Could not create the U-Boot image."
	exit 4
fi

if [ ! -z $1  ] && [ "$1" = "tftp" ]
then
	echo -e "Copying the kernel to the tftp_dir, now.\n---------------------\n"
	sudo cp arch/arm/boot/uImage $tftp_dir/uImage.buffalo
	sleep 3
fi

if [ ! -z $1  ] && [ "$1" = "install" ]
then
	echo -e "Creating a kernel tar archive.\n---------------------\n"
	if [ ! -d $dest_dir/tmp/ ]
	then
		mkdir -p $dest_dir/tmp/
	fi
	cp .config $dest_dir/tmp/kirkwood-lswvl.config
	cp arch/arm/boot/uImage $dest_dir/tmp/
	cp arch/arm/boot/zImage.fdt $dest_dir/tmp/
	cp arch/arm/boot/zImage $dest_dir/tmp/
	cp arch/arm/boot/dts/kirkwood-lswvl.dtb $dest_dir/tmp/
	$make_cmd INSTALL_MOD_PATH=$dest_dir/tmp/ modules_install
	$make_cmd INSTALL_HDR_PATH=$dest_dir/tmp/ headers_install
	cd $dest_dir/tmp/
	tar -cpJvf ../kirkwood-ls-wvl-1.1-`date +"%s"`.tar.xz .
	cd -
	rm -rf $dest_dir/tmp/*
fi

echo -e "---------------------------------\n
------ SUCCESSFULLY DONE !!! ----\n
---------------------------------\n"

exit 0
