## prod stage
FROM ubuntu

RUN apt-get update
RUN apt-get upgrade


RUN useradd -d /home/maprota -Um rota
RUN mkdir -p /home/maprota && chown -R rota:rota /home/maprota
RUN apt install -y build-essential cmake gcc-multilib libssl-dev wget python3

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

#todo hier noch server aufrufen
CMD [ "/bin/bash" ]
