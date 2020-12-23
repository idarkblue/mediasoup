#!/usr/bin/env bash

PMS_ROOT=/usr/local/pms/
if [ !-d $PMS_ROOT ]; then
    mkdir $PMS_ROOT
fi

if [ !-d $PMS_ROOT/record ]; then
    mkdir $PMS_ROOT/record
fi

if [ !-d $PMS_ROOT/bin ]; then
    mkdir $PMS_ROOT/bin
fi

cp out/Release/mediasoup-* $PMS_ROOT/bin/

if [ !-d $PMS_ROOT/conf ]; then
    cp -r conf $PMS_ROOT/
fi

if [ !-d $PMS_ROOT/certs ]; then
    cp -r certs/ $PMS_ROOT/
fi

if [ !-f /lib/systemd/system/pms.service ]; then
    cp pms.service /lib/systemd/system/pms.service
fi