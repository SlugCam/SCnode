// video_server.js
// ===============
//
// This file contains the code for the SWEETnet video server. It is responsible
// for receiving and storing video from the cameras.
//
// Format of protocol:
//
// - Camera Name (null terminated string)
// - Video ID (unsigned 32 int)
// - Video Length (unsigned 32 int)
// - Data

// We use the Node.js `net` library to facilitate TCP communication.
var net = require('net');



// Build a server object, `net.createServer` creates a server object and sets
// the function to be the listener for the `connection` event. This function
// receives as an argument a connection object which is an instance of
// `net.Socket`.
var server = net.createServer(function(c) { //'connection' listener
    console.log('server connected');

    //c.pipe(process.stdout);
    c.on('data', function(data) {
        process.stdout.write(data);
    })

    c.on('end', function() {
        // TODO clean up?
        console.log('server disconnected');
    });

});

server.listen(process.argv[2], function() {
    console.log('server bound to port ' + process.argv[2]);
});
