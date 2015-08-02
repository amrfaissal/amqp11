#include "amqp11.hpp"
#include <iostream>

auto main() -> int
{
	try
	{
		AMQP amqp("localhost:5672");

		auto qu2 = amqp.createQueue();
		qu2->Declare("q2");

		while (1)
		{
			qu2->Get(AMQP_NOACK);

			AMQPMessage * m = qu2->getMessage();

			std::cout << "count: " << m->getMessageCount() << std::endl;
			if (m->getMessageCount() > -1)
			{
				uint32_t j = 0;
				std::cout << "message\n" << m->getMessage(&j) << "\nmessage key: "
						<< m->getRoutingKey() << std::endl;
				std::cout << "exchange: " << m->getExchange() << std::endl;
				std::cout << "Content-type: " << m->getHeader("Content-type") << std::endl;
				std::cout << "Content-encoding: " << m->getHeader("Content-encoding") << std::endl;
			}
			else
				break;

		}
	} catch (AMQPException & e)
	{
		std::cout << e.getMessage() << std::endl;
	}

	return 0;
}
