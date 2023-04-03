<<<<<<< HEAD
scons -j4
=======
scons
>>>>>>> 8a24113c51a63a2243b0b64194fcd9f1539803b0
cp build/fmt_chuanyun-f2s.bin package_bin/
cd package_bin/
./joint2flash_cf.sh
cp -f fmt_chuangyun_f2s_pkg.bin ../../../../
