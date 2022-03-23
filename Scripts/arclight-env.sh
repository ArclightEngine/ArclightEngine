#!/bin/sh

export ARCLIGHT_ROOT=$(dirname $(readlink -f "$0"))/..
export PATH="$PATH:$ARCLIGHT_ROOT"

