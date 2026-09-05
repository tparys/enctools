# ENC Tools & Chart Server

## Overview

ENC Tools provides a simple visualization of ENC(S-57) chart data. It includes a
basic WTMS tile server compatible with various GIS and web mapping tools such as
Leaflet.

The application is currently Linux native, and targets an Ubuntu 24.04 Operating
System. Other environments may be added as time and interest permits.

## Quick Start

1. Install dependencies

```
$ sudo apt install cmake libcairo2-dev libgdal-dev libgtest-dev libmicrohttpd-dev libcgal-dev libjsoncpp-dev
```

2. Compile the software

```
~ $ cd enctools
enctools $ mkdir build
enctools $ cd build
build $ cmake ..
build $ make -j$(nproc)
build $ make test
```

3. Obtain a set of ENC(S-57) charts, such as from NOAA ENC Chart Downloader

4. Create a local user configuration and point ENC Tools' Tile Server at them

```
build $ cd ..
enctools $ rm -rf ~/.enctools
enctools $ mkdir ~/.enctools
enctools $ cp share/enctools/config.json ~/.enctools
enctools $ gedit ~/.enctools/config.json
```

```
{
    "data":
    {
        "chart-dir": "/path/to/charts", // <- Edit this line
```

5. Start the tile server (be patient on first start)

```
enctools $ cd build
build $ ./bin/enc_tile_server
Using config directory: /home/user/.enctools ...
Using share directory: /home/user/dev/git/enctools/build/share/enctools ...
Using config file: /home/user/.enctools/config.json
 - Charts: /home/user/Downloads/data/maps/ENC/NOAA
 - Metadata: /home/user/.enctools/meta
 - Theme: /home/user/dev/git/enctools/build/share/enctools/color-table.json
 - Styles: /home/user/dev/git/enctools/build/share/enctools/styles
 - Tile Size: 256
 - Scale Base: 5.92e+08
7133 charts loaded
 - Default land loaded: /home/user/Downloads/data/gshhg/GSHHS_shp/l/GSHHS_l_L1.shp [GSHHS_l_L1]
Loaded 63 colors across 3 themes
Loaded: standard-day
Loaded: base-day
Loaded: standard-dusk
Loaded: base-dusk
Loaded: standard-night
Loaded: base-night
```

6. Point your GIS or Web Map application at ENC Tools' Tile Server

```
http://127.0.0.1:8888/base-day/{z}/{y}/{x}.png
```

7. Scroll around and enjoy.
