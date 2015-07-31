#include "../include/amqp++.h"

using namespace std;

auto main() -> int
{
	try
	{
		AMQP amqp("localhost:5672");

		AMQPQueue * qu2 = amqp.createQueue("q2");
		qu2->Declare();
		qu2->Bind("e", "");
		qu2->setConsumerTag("tag_123");

		int i = 0;
		qu2->addEvent(AMQPEventType::AMQP_MESSAGE,
				[&](AMQPMessage * message) -> int
				{
					uint32_t j = 0;
					char * data = message->getMessage(&j);
					if (data)
					cout << data << endl;

					i++;

					cout << "#" << i << " tag="<< message->getDeliveryTag() << " content-type:"<< message->getHeader("Content-type");
					cout << " encoding:"<< message->getHeader("Content-encoding")<< " mode="<<message->getHeader("Delivery-mode")<<endl;

					if (i > 10)
					{
						AMQPQueue * q = message->getQueue();
						q->Cancel( message->getConsumerTag() );
					}
					return 0;
				});

		qu2->addEvent(AMQPEventType::AMQP_CANCEL, [&](AMQPMessage * message) -> int
		{
			cout << "cancel tag="<< message->getDeliveryTag() << endl;
			return 0;
		});

		qu2->Consume(AMQP_NOACK);
	} catch (AMQPException & e)
	{
		std::cout << e.getMessage() << std::endl;
	}

	return 0;
}
