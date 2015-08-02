#include <amqp11.hpp>

auto main() -> int
{
	try
	{
		AMQP amqp("localhost:5672");

		auto qu2 = amqp.createQueue();
		qu2->Declare();
		qu2->Bind("ex1", "");

		int i = 0;
		qu2->addEvent(AMQPEventType::AMQP_MESSAGE,
				[&](AMQPMessage * message) -> int
				{
					uint32_t j = 0;
					std::string data = message->getMessage(&j);
					if (!data.empty()) {
						i++;

						std::cout << "Received data: " << data;
						std::cout << " |" << i << ": Tag="<< message->getDeliveryTag()
											<< " / Content-type:"<< message->getHeader("Content-type");
						std::cout << " / Encoding: "<< message->getHeader("Content-encoding")
											<< " / Mode="<< message->getHeader("Delivery-mode")<< std::endl;
					}
					return 0;
		});

		qu2->addEvent(AMQPEventType::AMQP_CANCEL, [&](AMQPMessage * message) -> int
		{
			std::cout << "Cancel tag=" << message->getDeliveryTag() << std::endl;
			return 0;
		});

		qu2->Consume(AMQP_NOACK);
	} catch (AMQPException & e) {
		std::cout << e.getMessage() << std::endl;
	}

	return 0;
}
