## prod stage
FROM ubuntu

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update --assume-yes && apt-get upgrade --assume-yes

RUN mkdir -p /home/maprota && useradd -d /home/maprota -Um rota
RUN apt install --assume-yes build-essential cmake gcc-multilib libssl-dev wget python3

WORKDIR /home/maprota

COPY --chown=rota:rota . .

RUN mkdir build && chown -R rota:rota build

## install boost
RUN ./install_boost.sh

## make/install rota
WORKDIR /home/maprota/build
RUN cmake ..
RUN make -j$(nproc)
RUN make install
USER rota

CMD [ "SquadMaprotaServer" ]
