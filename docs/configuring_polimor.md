Configuring PoliMOR
===================
In this doc we describe how to configure PoliMOR agents and message queues.

1. Configuration for agents and queues
--------------------------------------
The configuration for agents and queues is input using a config.yaml file (example provided in docs/example_configs). A config.yaml file has two sections: messaging service and agents. In the messaging service the name of the backend messaging system used, messaginging server host and port, details of the queues including the names of streams, consumers, subjects, number of stream replicas are specified. For instance in the example config.yaml file provided the backend messaging system is 'nats' and messaging server listens to localhost:4222, three message queues initialized with 'scan', 'purge', and 'migration' being their stream names. For each stream the consumer, subject and number of replicas are also specified.
Streams and consumers can have more advanced configurations as indicated in the contrib/schema.json file. Some of the additional options for streams are max messages in the queue, max age of messages, storage type, discard policy etc. The types and default values for these options are specified in the schema.json file. The values specified here can be overridden if new values are input from the config.yaml file.
Since PoliMOR queues are implemented using NATS message queues the detailed description for different configuration options for the streams and consumers can be can be found here: https://docs.nats.io/nats-concepts/jetstream/streams, https://docs.nats.io/nats-concepts/jetstream/consumers . 


In the agents section details about various agents in PoliMOR are given. 4 types of agents initialized in the example file are 'scan_agents', 'policy_agents', 'purge_agents' and 'migration_agents'. For each agent options to input its id, interval, root directory and interacting queues are given.




2. Configuring NATS server
--------------------------
The backend messaging system used by PoliMOR is NATS. For better resiliency and load balacing a three or five NATS server cluster can be used to run PoliMOR. Each NATS server has a configuration file (example given in docs/example_configs). We have populated the config file with the required field names and their recommended settings. More details about NATS server configurations can be found here:  https://docs.nats.io/running-a-nats-service/configuration


3. Starting the agents 
----------------------
Providing -h option with agents displays all flags and options that the agents accepts through command line. Agents can be started by specifying their id flag. All other options can be specified in the config.yaml file for convenience.
