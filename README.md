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

    var flumelog = require('flume_eio');
    var fl = new flumelog.FlumeLogEio('localhost', 35853);
    fl.log("message");

### AgentSink
To set up your agent sink in flume, it will look like:

    host : thrift($port) | agentBESink("localhost",35853)
