FROM ubuntu:18.04

# to reduce image size all build and cleanup steps are performed in one docker layer
RUN apt-get -y update && apt-get -y install git cmake libsdl2-dev libsdl2-image-dev

RUN git clone --recurse-submodules https://github.com/notbaab/HEAT.git
WORKDIR /HEAT
RUN mkdir build
RUN cd build && cmake .. && make -j

ENTRYPOINT ["/HEAT/build/server"]
