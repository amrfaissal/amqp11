#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <array>

#include "amqp11.hpp"
#include "SynchronizedQueue.h"


std::array<int, 3> ports {
    {5672, 5673, 5674}
};

typedef std::string  protocol_t;
SynchronizedQueue<protocol_t> m_q[1];


int onCancel(AMQPMessage * message ) {
    std::cout << "cancel tag="<< message->getDeliveryTag() << std::endl;
    return 0;
}

int i = 0;
int onMessage(AMQPMessage * message) {
    i++;
    uint32_t j = 0;
    const char * data = message->getMessage(&j);
    if (data) {
        ISynchronizedQueue<protocol_t>* pQ = (ISynchronizedQueue<protocol_t>*) m_q;
        pQ->add(data);
    }
};


int counter = 0;

void handle() {
    ISynchronizedQueue<protocol_t>* pQ = (ISynchronizedQueue<protocol_t>*)m_q;
    while(true)
    {
        boost::this_thread::sleep( boost::posix_time::milliseconds(50) );

        protocol_t  protocol;
        while (pQ->get(protocol)) {
            counter++;
            std::cout << protocol << std::endl;
        }
    }
}

int reconnects = 0;
void consume()
{
    try
    {
        reconnects++;
        std::cout << "Connecting:" << reconnects << "..." << std::endl;

        srand((unsigned)time(0));
        std::stringstream ss;
        ss << "localhost:" << ports[rand() % 3];

        AMQP amqp(ss.str());

        AMQPQueue * queue = amqp.createQueue("hello-queue");
        queue->Declare();
        queue->Bind( "hello-exchange", "hola");

        std::cout << "Connected." << std::endl;
        reconnects = 0;

        queue->setConsumerTag("hello-consumer");
        queue->addEvent(AMQP_MESSAGE, onMessage );
        queue->addEvent(AMQP_CANCEL, onCancel );

        queue->Consume(AMQP_NOACK);

        throw AMQPException("Something happened in Consume");
    } catch (AMQPException & e) {
        std::cout << e.getMessage() << std::endl;
        boost::this_thread::sleep( boost::posix_time::milliseconds(3000) );
        consume();
    }
}


auto main() -> int
{
    boost::thread_group threads;

    threads.create_thread(handle);
    threads.create_thread(consume);

    // Wait for Threads to finish
    threads.join_all();

    return 0;
}
