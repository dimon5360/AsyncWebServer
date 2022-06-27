
// #include "KafkaProcess.h"

// #include <iostream>
// #include <string>

// #include <stdexcept>
// #include <iostream>
// #include <csignal>
// #include <boost/program_options.hpp>
// #include "cppkafka/consumer.h"
// #include "cppkafka/configuration.h"

// // #include <boost/asio.hpp>

// // #include <librdkafka/rdkafkacpp.h>
// // #include <librdkafka/rdkafka.h>

// #include <cppkafka/cppkafka.h>

// void KafkaProcess::CreateKafkaConsumer()
// {
//     std::cout << __func__ << "\n";

//     using namespace cppkafka;
//     using namespace std;

//     string brokers = "127.0.0.1:9092";
//     string topic_name = "test-topic";
//     string group_id = "test-consumer-group";

//     // Construct the configuration
//     Configuration config = {
//         {"bootstrap.servers", brokers},
//         {"group.id", group_id},
//         // Disable auto commit
//         {"enable.auto.commit", false}};

//     // Create the consumer
//     Consumer consumer(config);

//     // Print the assigned partitions on assignment
//     consumer.set_assignment_callback([](const TopicPartitionList &partitions)
//                                      { cout << "Got assigned: " << partitions << endl; });

//     // Print the revoked partitions on revocation
//     consumer.set_revocation_callback([](const TopicPartitionList &partitions)
//                                      { cout << "Got revoked: " << partitions << endl; });

//     // Subscribe to the topic
//     consumer.subscribe({topic_name});

//     cout << "Consuming messages from topic " << topic_name << endl;

//     // Now read lines and write them into kafka
//     while (true)
//     {
//         // Try to consume a message
//         Message msg = consumer.poll();
//         if (msg)
//         {
//             // If we managed to get a message
//             if (msg.get_error())
//             {
//                 // Ignore EOF notifications from rdkafka
//                 if (!msg.is_eof())
//                 {
//                     cout << "[+] Received error notification: " << msg.get_error() << endl;
//                 }
//             }
//             else
//             {
//                 // Print the key (if any)
//                 if (msg.get_key())
//                 {
//                     cout << msg.get_key() << " -> ";
//                 }
//                 // Print the payload
//                 cout << "Received: " << msg.get_payload() << endl;
//                 // Now commit the message
//                 consumer.commit(msg);
//             }
//         }
//     }
// }

// void KafkaProcess::CreateKafkaProducer()
// {
//     std::cout << __func__ << "\n";
//     using namespace cppkafka;
//     using namespace std;

//     string brokers = "127.0.0.1:9092";
//     string topic_name = "test-topic";
//     int partition_value = 1;

//     // Create a message builder for this topic
//     MessageBuilder builder(topic_name);

//     // Construct the configuration
//     Configuration config = {
//         { "bootstrap.servers", brokers }
//     };

//     // Create the producer
//     Producer producer(config);

//     cout << "Producing messages into topic " << topic_name << endl;

//     // Now read lines and write them into kafka
//     string line = "hello";
//     while (getline(cin, line)) {
//         // Set the payload on this builder
//         builder.payload(line);

//         // Actually produce the message we've built
//         producer.produce(builder);
//     }
    
//     // Flush all produced messages
//     producer.flush();
// }

// KafkaProcess::KafkaProcess()
// {
//     std::cout << "Construct KafkaProcess class\n";
//     auto t1 = std::thread{ [&] () { CreateKafkaProducer(); }};
//     auto t2 = std::thread{ [&] () { CreateKafkaConsumer(); }};

//     t1.join();
//     t2.join();
//     // CreateKafkaConsumer();
// }

// KafkaProcess::~KafkaProcess()
// {
//     std::cout << "Destruct KafkaProcess class\n";
// }