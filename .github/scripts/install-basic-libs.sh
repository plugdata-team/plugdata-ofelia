#!/usr/bin/env bash

export DEBIAN_FRONTEND=noninteractive
export TZ="Europe/Amsterdam"

apt install -y build-essential ccache clang cmake curl git gstreamer1.0-plugins-bad gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly libasound2-dev libassimp-dev libcairo-dev libcurl4-openssl-dev libfreetype6-dev libglu1-mesa-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-0 libjack-jackd2-dev libudev-dev libunwind-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev lsb-release mesa-common-dev pax-utils pkg-config python3 python3-pip rsync sudo wget

apt install --ignore-missing adwaita-icon-theme at-spi2-core default-libmysqlclient-dev freeglut3 \
freeglut3-dev gconf-service gconf2-common gdal-data gfortran gfortran-8 \
gir1.2-atk-1.0 gir1.2-atspi-2.0 gir1.2-freedesktop gir1.2-gdkpixbuf-2.0 \
gir1.2-gtk-3.0 gir1.2-harfbuzz-0.0 gir1.2-pango-1.0 gtk-update-icon-cache \
hicolor-icon-theme ibverbs-providers icu-devtools javascript-common libaacs0 \
libaec0 libarmadillo9 libarpack2 libatk-bridge2.0-0 libatk-bridge2.0-dev \
libatk1.0-0 libatk1.0-data libatk1.0-dev libatspi2.0-0 libatspi2.0-dev \
libavahi-client3 libavahi-common-data libavahi-common3 libavcodec-dev \
libavformat-dev libavformat58 libavresample-dev libavresample4 libavutil-dev \
libbdplus0 libblas3 libbluray2 libboost-filesystem-dev \
libboost-filesystem1.67-dev libboost-filesystem1.67.0 \
libboost-system1.67-dev libboost-system1.67.0 libboost1.67-dev \
libcaf-openmpi-3 libcharls2 libcoarrays-dev libcoarrays-openmpi-dev \
libcolord2 libcups2 libdap25 libdapclient6v5 libdapserver7v5 libdbus-1-dev \
libdbus-glib-1-2 libdc1394-22-dev libepoxy-dev libepoxy0 libepsilon1 \
libevent-2.1-6 libevent-core-2.1-6 libevent-pthreads-2.1-6 libexif-dev \
libexif-doc libexif12 libfabric1 libflac-dev libfreeimage-dev libfreeimage3 \
libfreexl1 libfribidi-dev libfyba0 libgconf-2-4 libgd3 libgdal20 \
libgdcm2-dev libgdcm2.8 libgdk-pixbuf2.0-dev libgeos-3.7.1 libgeos-c1v5 \
libgeotiff2 libgfortran-8-dev libgfortran5 libgif7 libgl2ps1.4 libglew-dev \
libglew2.1 libglfw3 libglfw3-dev libgmp-dev libgmpxx4ldbl libgnutls-dane0 \
libgnutls-openssl27 libgnutls28-dev libgnutlsxx28 libgphoto2-6 \
libgphoto2-dev libgphoto2-l10n libgphoto2-port12 libgraphite2-dev libgtk-3-0 \
libgtk-3-bin libgtk-3-common libgtk-3-dev libharfbuzz-dev \
libharfbuzz-gobject0 libharfbuzz-icu0 libhdf4-0-alt libhdf5-103 \
libhdf5-openmpi-103 libhwloc-dev libhwloc-plugins libhwloc5 libibverbs-dev \
libibverbs1 libicu-dev libidn2-dev libilmbase-dev libjbig-dev libjpeg-dev \
libjpeg62-turbo-dev libjs-jquery libjson-glib-1.0-0 libjson-glib-1.0-common \
libjxr0 libkmlbase1 libkmlconvenience1 libkmldom1 libkmlengine1 \
libkmlregionator1 libkmlxsd1 liblapack3 liblept5 libmariadb-dev \
libmariadb-dev-compat libmariadb3 libnetcdf-c++4 libnetcdf13 libnl-3-200 \
libnl-3-dev libnl-route-3-200 libnl-route-3-dev libnspr4 libnss3 libnuma-dev \
libodbc1 libogdi3.2 libogg-dev libopenal-dev libopencv-calib3d-dev \
libopencv-calib3d3.2 libopencv-contrib-dev libopencv-contrib3.2 \
libopencv-core-dev libopencv-core3.2 libopencv-dev libopencv-features2d-dev \
libopencv-features2d3.2 libopencv-flann-dev libopencv-flann3.2 \
libopencv-highgui-dev libopencv-highgui3.2 libopencv-imgcodecs-dev \
libopencv-imgcodecs3.2 libopencv-imgproc-dev libopencv-imgproc3.2 \
libopencv-ml-dev libopencv-ml3.2 libopencv-objdetect-dev \
libopencv-objdetect3.2 libopencv-photo-dev libopencv-photo3.2 \
libopencv-shape-dev libopencv-shape3.2 libopencv-stitching-dev \
libopencv-stitching3.2 libopencv-superres-dev libopencv-superres3.2 \
libopencv-ts-dev libopencv-video-dev libopencv-video3.2 \
libopencv-videoio-dev libopencv-videoio3.2 libopencv-videostab-dev \
libopencv-videostab3.2 libopencv-viz-dev libopencv-viz3.2 libopencv3.2-java \
libopencv3.2-jni libopenexr-dev libopenmpi-dev libopenmpi3 libp11-kit-dev \
libpango1.0-dev libpangoxft-1.0-0 libpmix2 libpoco-dev libpococrypto60 \
libpocodata60 libpocodatamysql60 libpocodataodbc60 libpocodatasqlite60 \
libpocoencodings60 libpocofoundation60 libpocojson60 libpocomongodb60 \
libpoconet60 libpoconetssl60 libpocoredis60 libpocoutil60 libpocoxml60 \
libpocozip60 libpoppler82 libpq5 libproj13 libpsm-infinipath1 libpsm2-2 \
libpugixml-dev libpugixml1v5 libpulse-dev libpulse-mainloop-glib0 \
libpython2.7 libqhull7 libraw1394-dev libraw1394-tools libraw19 librdmacm1 \
librest-0.7-0 librtaudio-dev librtaudio6 libsndfile1-dev libsocket++1 \
libsoup-gnome2.4-1 libspatialite7 libsqlite3-dev libssh-gcrypt-4 libssl-dev \
libsuperlu5 libswresample-dev libswscale-dev libswscale5 libsz2 \
libtasn1-6-dev libtasn1-doc libtbb-dev libtbb2 libtcl8.6 libtesseract4 \
libtiff-dev libtiffxx5 libtk8.6 libunbound8 liburiparser-dev liburiparser1 \
libusb-1.0-0-dev libusb-1.0-doc libvorbis-dev libvtk6.3 libvulkan-dev \
libxerces-c3.2 libxft-dev libxft2 libxi-dev libxkbcommon-dev libxkbcommon0 \
libxmu-dev libxmu-headers libxmu6 libxpm4 libxss1 libxt-dev libxt6 \
libxtst-dev mariadb-common mysql-common nettle-dev ocl-icd-libopencl1 \
odbcinst odbcinst1debian2 opencv-data openmpi-bin openmpi-common \
pango1.0-tools poppler-data proj-bin proj-data wayland-protocols \
x11proto-input-dev x11proto-record-dev xkb-data \
gstreamer1.0-alsa gstreamer1.0-libav gstreamer1.0-pulseaudio libavfilter7 \
libmysofa0 libnorm1 libpgm-5.2-0 libpostproc55 librubberband2 libsodium23 \
libvidstab1.1 libzmq5 \

pip3 install copydeps