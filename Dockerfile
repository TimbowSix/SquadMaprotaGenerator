## build stage
FROM ubuntu as build

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update --assume-yes &&\
    apt-get upgrade --assume-yes &&\
    apt install --assume-yes build-essential cmake gcc-multilib libssl-dev wget python3

RUN mkdir -p /home/maprota && useradd -d /home/maprota -Um rota

WORKDIR /home/maprota
RUN mkdir build && chown -R rota:rota build

## install boost
COPY --chown=rota:rota ./install_boost.sh .
RUN ./install_boost.sh

## make/install rota
COPY --chown=rota:rota . .
WORKDIR /home/maprota/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make package -j$(nproc)

## runtime stage
FROM ubuntu as runtime

RUN apt-get update --assume-yes &&\
    apt-get upgrade --assume-yes &&\
    apt install --assume-yes libboost-all-dev

RUN mkdir -p /home/maprota && useradd -d /home/maprota -Um rota

WORKDIR /home/maprota
COPY --from=build /home/maprota/build/SquadMaprotaGenerator.deb .
RUN apt install ./SquadMaprotaGenerator.deb
USER rota

CMD [ "SquadMaprotaServer" ]
