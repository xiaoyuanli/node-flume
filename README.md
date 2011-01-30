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
    node-waf configure && node-waf build
