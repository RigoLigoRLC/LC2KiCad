FROM ubuntu as builder

RUN apt-get update -y && apt-get -y install cmake g++

COPY . /lc2kicad

WORKDIR /lc2kicad

RUN mkdir -p build && cd build && cmake .. && make -j 8

FROM alpine as prod

RUN apk --no-cache add libc6-compat libstdc++

WORKDIR /data

COPY --from=0 /lc2kicad/build/lc2kicad /opt

ENTRYPOINT ["/opt/lc2kicad"]
