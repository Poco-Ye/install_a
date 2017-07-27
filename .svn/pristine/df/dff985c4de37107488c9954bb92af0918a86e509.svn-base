0) to build a pc version install_a:
  $ make -f Makefile.pc
  - it will create install_a_pc 

1) to test NAND, create the following:
   $ touch /tmp/mtd0 /tmp/mtdblock0
   $ chmod 777 /tmp/mtd0 /tmp/mtdblock0

2) to test EMMC, create the following:
   $ touch /tmp/mmcblk0
   $ chmod 777 /tmp/mmcblk0

3) generate setting.txt:
   a) ./parse_nand_list.pl flash_nand_list.h > Rconfig
      - it will parse flash_nand_list.h, and create Rconfig, which will be used by mconf.
      - flash_nand_list.h is from bootcode.

   b) ./mconf
      - it read Rconfig, and display a menu, so that one can choose the required flash.
      - it will create setting.txt
      - mconf is from image_file_creator/components/installer_x86/

4) ./install_a_pc install.img
   - it will read setting.txt, to get the NAND flash info.


/sys/realtek_boards/system_parameters
bootcode_start=0 bootcode_size=0x1700000 factory_start=0x1700000 factory_size=0x400000 boot_flash=nand
----------------------------------
output:
/tmp/str.txt     <==  /sys/realtek_boards/system_parameters
/tmp/config.txt  <==  install.img, config.txt
/tmp/factory.tar
/tmp/factory/
/tmp/package5/

