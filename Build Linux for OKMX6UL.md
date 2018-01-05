### 目标
在飞凌开发板OKMX6UL上建立运行在SD卡上的Linux系统，包括uboot, kernel, rootfs
参考：
https://eewiki.net/display/linuxonarm/MCIMX6UL-EVK
### u-boot
Robert Nelson用的u-boot是https://github.com/u-boot/u-boot
可能存在无法在SD卡上启动的问题，选择nxp官方u-boot
***
	git clone http://git.freescale.com/git/cgit.cgi/imx/uboot-imx.git
	cd uboot-imx
	git checkout origin/imx_v2017.03_4.9.11_1.0.0_ga -b imx_v2017.03_4.9.11_1.0.0_ga
	patch -p1 < 0002-uboot-linux.patch
	export CC={cross compile path, eg: /.../gcc-linaro-6.4.1-2017.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-}
	make ARCH=arm CROSS_COMPILE=${CC} distclean
	make ARCH=arm CROSS_COMPILE=${CC} mx6ul_14x14_evk_defconfig
	make ARCH=arm CROSS_COMPILE=${CC}
成功之后会在uboot-imx目录下生成u-boot-dtb.imx
### Kernel
参考Robert Nelson的方法可以生成内核镜像
***
	git clone https://github.com/RobertCNelson/armv7-multiplatform
	cd armv7-multiplatform/
	git checkout origin/v4.9.x -b tmp
	./build_kernel.sh
### Root File System
#### Debian 9
Download:

	~/
	wget -c https://rcn-ee.com/rootfs/eewiki/minfs/debian-9.3-minimal-armhf-2017-12-09.tar.xz
Verify:

	~/
	sha256sum debian-9.3-minimal-armhf-2017-12-09.tar.xz
	5120fcfb8ff8af013737fae52dc0a7ecc2f52563a9aa8f5aa288aff0f3943d61  debian-9.3-minimal-armhf-2017-12-09.tar.xz

Extract:

	~/
	tar xf debian-9.3-minimal-armhf-2017-12-09.tar.xz

### Setup SD Card
手动分区，fdisk /dev/sdb，只需要一个主分区，需要预留1M的空间给uboot
***
With util-linux v2.26, sfdisk was rewritten and is now based on libfdisk.

	sudo sfdisk --version
sfdisk from util-linux 2.27.1

	sfdisk >= 2.26.x
	sudo sfdisk /dev/sdb <<-__EOF__
	1M,,L,*
	__EOF__

sfdisk <= 2.25.x

	sudo sfdisk --unit M /dev/sdb <<-__EOF__
	1,,L,*
	__EOF__

格式化:

	sudo mkfs.ext4 -L rootfs /dev/sdb1

### uboot写入SD卡

	sudo dd if=u-boot-dtb.imx of=/dev/sdb bs=1k seek=1
### kernel & rootfs写入SD卡
参考Robert Nelson的方法即可
