#include "../include/amqp++.h"

using namespace std;

auto main() -> int
{
	try
	{
		AMQP amqp("localhost:5672");

		AMQPExchange * ex = amqp.createExchange("e");
		ex->Declare("e", ExchangeType::FANOUT);

		AMQPQueue * qu2 = amqp.createQueue("q2");
		qu2->Declare();
		qu2->Bind("e", "");

		string ss = "message 1";

		ex->setHeader("Delivery-mode", 2);
		ex->setHeader("Content-type", "text/text");
		ex->setHeader("Content-encoding", "UTF-8");

		ex->Publish(ss, "");

		ex->Publish("message 2", "");
		ex->Publish("message 3", "");

	} catch (AMQPException & e)
	{
		std::cout << e.getMessage() << std::endl;
	}

	return 0;

}
