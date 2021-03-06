cd src
make xz
cd ..
tar -c --xz --exclude=nbproject --exclude=.git --exclude=*.png --exclude=cfrtil-gdb  --exclude=*.o --exclude *.kdev* -f cfrtil.tar.xz * .init.cft
cp cfrtil.tar.xz cfrtil.$1.tar.xz
cp *.tar.xz ~/backup/archive
cp *.tar.xz ~/Public
cp *.tar.xz ~/ubackup 
cp *.tar.xz ~/Documents/backup/
cp *.tar.xz ~/Programs/Programs2/forth/cfrtil/backup/
cp *.tar.xz ~/Programs/Programs2/backup/
cp *.tar.xz ~/workspace/backup/
cp *.tar.xz ~/media/sda13/backup
cp *.tar.xz ~/Public/archive
cp *.tar.xz ~/Public/backup
mv *tar.xz ~/backup/
ls -al ~/backup/cfrtil.$1.tar.xz
ls -al ~/backup/archive/cfrtil.$1.tar.xz
