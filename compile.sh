# bitbake core-image-full-cmdline -c populate_sdk로 sdk를 설치할 수 있음 /opt/gcc-beaglebone에 설치...
# 혹은 export CC=arm-linux-gnueabihf-gcc로 설정해도 됨
source /opt/gcc-beaglebone/environment-setup-armv7at2hf-neon-poky-linux-gnueabi

#export CC=arm-linux-gnueabihf-gcc

export CFLAGS=""
export CPPFLAGS=""
export LDFLAGS=""

export SERVER_TARGET="server"
export CLIENT_TARGET="client"

export SERVER_SOURCE="$SERVER_TARGET""_main.c"
export CLIENT_SOURCE="$CLIENT_TARGET""_main.c"

export SCR="client.c server.c"

make -j