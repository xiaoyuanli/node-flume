## Node-Flume
A C++ async Flume interface to Node.js.

### Flume
Flume is a distributed, reliable, and available service for efficiently collecting, aggregating, and moving large amounts of log data. It has a simple and flexible architecture based on streaming data flows. It is robust and fault tolerant with tunable reliability mechanisms and many failover and recovery mechanisms. The system is centrally managed and allows for intelligent dynamic management. It uses a simple extensible data model that allows for online analytic applications.
http://www.cloudera.com/hadoop

### Installing
Ensure the following packages are installed:

* Node.js
* Thrift

#### Building
Run the following commands to build the modules.

    cd src && node-waf configure && node-waf build

Your extension will then be located in _src/build/default/flume_eio.node_.

### Example
In order to use flume node

    var flumelogger = require('flume');
    // var fl = new flumelogger.FlumeLog('localhost', 35853);
    // Values are hard-coded for now as localhost and 35854
    var f = new flumelogger.FlumeLog()
    f.log("my message");
    //  Use optional tags
    var flumeTags = {"type": "testMessage"};
    f.log("message 2",flumeTags);

### AgentSink
To set up your agent sink in flume, it will look like:

    host : rpcSource(35854) | agentBESink("localhost",35853)
