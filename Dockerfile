FROM gcc
RUN  apt-get update && \
     apt-get -y install ca-certificates curl wget git pkg-config libssl-dev libsasl2-dev

RUN mkdir -p /usr/src
WORKDIR /usr/src

RUN  wget https://github.com/mongodb/mongo-c-driver/releases/download/1.9.3/mongo-c-driver-1.9.3.tar.gz && \
     tar xzf mongo-c-driver-1.9.3.tar.gz && \
     cd mongo-c-driver-1.9.3 && \
     ./configure --disable-automatic-init-and-cleanup && \
     make && \
     make install

RUN  wget -O mongowatch.tar.gz https://github.com/dtravin/mongowatch/archive/master.tar.gz && \
     tar xzf mongowatch.tar.gz

WORKDIR /usr/src/mongowatch-master

RUN  gcc -o mongowatch mongowatch.c $(pkg-config --libs --cflags libmongoc-1.0)

ENV LD_LIBRARY_PATH=/usr/local/lib

CMD [ "./mongowatch", "mongodb://localhost:27017,localhost:27018,localhost:27019/?replicaSet=rs0", "test", "c1"]
