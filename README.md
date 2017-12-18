Proof of concept
Mongo 3.6 collection watcher using change stream API

### Dependencies
- Install latest libmongoc + libbson from source 
```$ git clone https://github.com/mongodb/mongo-c-driver && cd mongo-c-driver
$ ./autogen.sh
$ make
$ sudo make install
```

- Run mongo replicaset

### Build 
`gcc -o mongowatch mongowatch.c $(pkg-config --libs --cflags libmongoc-1.0)`

### Test
- Terminal 1 
```./mongowatch "mongodb://localhost:27017,localhost:27018,localhost:27019/test?replicaSet=rs0" test c1```

- Terminal 2 
```mongo test --eval 'db.c1.insert({});'```
