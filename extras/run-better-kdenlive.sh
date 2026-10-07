#!/bin/bash
# Run Better Kdenlive from a local install prefix, with its own config, data and cache,
# so it never touches the settings of a system-wide Kdenlive.
BK="${BK_DIR:-$HOME/better-kdenlive}"
export XDG_CONFIG_HOME="$BK/config"
export XDG_CACHE_HOME="$BK/cache"
export XDG_DATA_HOME="$BK/data"
export XDG_DATA_DIRS="$BK/install/share:/usr/local/share:/usr/share"
export LD_LIBRARY_PATH="$BK/install/lib64:$BK/install/lib:$LD_LIBRARY_PATH"
exec "$BK/install/bin/kdenlive" "$@"
