
RUN \
    apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y gzip bzip2 git cmake wget devscripts debhelper ruby ruby-dev rpm && \
    apt-get install -y build-essential libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev libglm-dev libxxf86vm-dev libfreeimage-dev pandoc wx3.0-headers libwxbase3.0-dev libwxgtk-media3.0-dev && \
    gem install fpm && \
    mkdir /build && \
    mkdir /pkg

VOLUME ["/script"]
VOLUME ["/build"]
VOLUME ["/pkg"]

WORKDIR /build
