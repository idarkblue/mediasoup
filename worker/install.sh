#!/usr/bin/env bash

DF_PATH=$1
PMS_ROOT=$1/$2

if [ ! -d $PMS_ROOT ]; then
    mkdir -p $PMS_ROOT
fi

if [ ! -d $PMS_ROOT/record ]; then
    mkdir $PMS_ROOT/record
fi

if [ ! -d $PMS_ROOT/bin ]; then
    mkdir $PMS_ROOT/bin
fi

cp out/Release/mediasoup-* $PMS_ROOT/bin/

if [ ! -d $PMS_ROOT/conf ]; then
    cp -r conf $PMS_ROOT/
fi

if [ ! -d $PMS_ROOT/certs ]; then
    cp -r certs/ $PMS_ROOT/
fi

if [ ! -d $DF_PATH/lib/systemd/system/ ]; then
    mkdir -p $DF_PATH/lib/systemd/system/
fi

if [ ! -f $DF_PATH/lib/systemd/system/pms.service ]; then
    cp pms.service $DF_PATH/lib/systemd/system/pms.service
    systemctl daemon-reload
fi

if [ ! -d $DF_PATH/usr/bin/ ]; then
    mkdir -p $DF_PATH/usr/bin/
fi

if [ ! -f $DF_PATH/usr/bin/ffmpeg ]; then
    cp deps/bin/ffmpeg $DF_PATH/usr/bin/
fi

if [ ! -d $DF_PATH/usr/local/bin/ ]; then
    mkdir -p $DF_PATH/usr/local/bin/
fi

if [ ! -f $DF_PATH/usr/local/bin/ffmpeg ]; then
    cp deps/bin/ffmpeg $DF_PATH/usr/local/bin/
fi

if [ ! -d $DF_PATH/etc/nginx/default.d/ ]; then
    mkdir -p $DF_PATH/etc/nginx/default.d/
fi

if [ ! -f $DF_PATH/etc/nginx/default.d/record.conf ]; then
    yum install -y nginx
    cp record.conf $DF_PATH/etc/nginx/default.d/
fi

