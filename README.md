# birbcam

NOTE: birbcam is currently in a broken state

Birbcam is intended detect birds in camera video footage and write 
the video along with bounding box metadata to a file. It's developed
for Nvidia Jetson using the NVIDIA DeepStream 4.0 framework.

## Requirements:
- [CMake](https://cgold.readthedocs.io/en/latest/first-step/installation.html#ubuntu)
- [Nvidia DeepStream 4.0](https://docs.nvidia.com/metropolis/deepstream/4.0/dev-guide/index.html)

## Building / installation
```git clone (repo)
cd birbcam
mkdir build
cd build
cmake ..
make
(and optionally)
(sudo) make install
```
(sudo is not required if the installation prefix is already
writable to the user)
## Usage:
```
 $ ./birbcam --help
Usage:
  birbcam [OPTION?] - Birbcam

Help Options:
  -h, --help                        Show help options
  --help-all                        Show all help options
  --help-gst                        Show GStreamer Options

Application Options:
  -o, --output=FILE                 output base filename (minus extension)

```

## Planned features:
- x86 Nvidia support
- secondary inference to classify detected birds
- support for IP cameras
- support for metadata dump formats other than json
- support for better backends (eg. kafka)