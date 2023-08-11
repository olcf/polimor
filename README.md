## PoliMOR

PoliMOR is a scalable, automated, and customizable policy engine framework for multi-tiered parallel file systems. It is composed of single-purpose agents that handle tasks such as gathering file metadata, making policy decisions, and then executing actions based on those policies. These agents are designed to communicate using distributed message queues, allowing the number of individual agents to be scaled up as needed. PoliMOR automates the data management tasks by precluding the need for admin intervention. The agents in PoliMOR can be customized to integrate any utilities/tools that perform tasks like metadata scanning and data placement management.

### Pre-requisites

To install and run PoliMOR you need to have the following pre-requisites.
1. NATS server (https://github.com/nats-io/nats-server)
2. NATS C client (https://github.com/nats-io/nats.c)
3. NATS command line tool (https://github.com/nats-io/natscli)
4. gcc-toolset-11



### Building and Installation

Follow these steps to build and install PoliMOR
1. Clone the repository
2. Enable gcc-toolset-11: scl enable gcc-toolset-11 /bin/bash
3. Run buildit script: ./buildit.sh


### QuickStart Guide
1. Start NATS servers: nats-server -c <nats-server.conf>
   More details about nats-server.conf can be found in docs directory.
2. Start agents:




### Authors
Chris Brumgard <br />
Anjus George <br />
Rick Mohr <br />
Ketan Maheshwari <br />
James Simmons <br />
Sarp Oral <br />


### LICENSE
Unless otherwise noted, the PoliMOR source files are distributed under the UT Battelle, LLC license found in the LICENSE file.
