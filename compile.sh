source /opt/gcc-beaglebone/environment-setup-armv7at2hf-neon-poky-linux-gnueabi

export CFLAGS=""
export CPPFLAGS=""
export LDFLAGS=""

export SERVER_TARGET="server"
export CLIENT_TARGET="client"

export SERVER_SOURCE="$SERVER_TARGET""_main.c"
export CLIENT_SOURCE="$CLIENT_TARGET""_main.c"

export SCR="client.c server.c"

make -j