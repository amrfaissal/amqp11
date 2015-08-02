#include "amqp11.hpp"

using std::string;
using std::map;


AMQPExchange::AMQPExchange(amqp_connection_state_t * cnn, int channelNum) {
	this->cnn = cnn;
	this->channelNum = channelNum;

	openChannel();
}

AMQPExchange::~AMQPExchange() { }

void AMQPExchange::Declare(string name) {
	this->name = name;
	this->params = 0;
	sendDeclareCommand();
}

void AMQPExchange::Declare(string name, ExchangeType type) {
	this->name = name;
	this->params = 0;
	this->type = type;
	sendDeclareCommand();
}

void AMQPExchange::Declare(string name, ExchangeType type, short params) {
	this->name = name;
	this->type = type;
	this->params = params;
	sendDeclareCommand();
}

const char * AMQPExchange::exchangetype_ctr(ExchangeType type) {
	switch (type)
	{
	case ExchangeType::DIRECT:
		return "direct";
	case ExchangeType::FANOUT:
		return "fanout";
	case ExchangeType::TOPIC:
		return "topic";
	}
	return "";
}

void AMQPExchange::sendDeclareCommand() {
	checkType();

	amqp_bytes_t exchange = amqp_cstring_bytes(name.c_str());
	amqp_bytes_t exchangetype = amqp_cstring_bytes(exchangetype_ctr(type));

	amqp_table_t args;
	args.num_entries = 0;
	args.entries = NULL;

	amqp_boolean_t passive 		=		(params & AMQP_PASSIVE)			? 1:0;
	amqp_boolean_t autodelete = 	(params & AMQP_AUTODELETE)	? 1:0;
	amqp_boolean_t durable 		=		(params & AMQP_DURABLE)			? 1:0;
	amqp_boolean_t internal		=		0;

	amqp_exchange_declare(*cnn, (amqp_channel_t) 1, exchange, exchangetype, passive, durable, autodelete, internal, args);

	amqp_rpc_reply_t res =amqp_get_rpc_reply(*cnn);

	AMQPBase::checkReply(&res);
}

void AMQPExchange::checkType() {
	short isErr = 1;
	if ( type == ExchangeType::DIRECT )
		isErr = 0;

	if ( type == ExchangeType::FANOUT)
		isErr = 0;

	if ( type == ExchangeType::TOPIC )
		isErr = 0;

	if (isErr) {
		throw AMQPException("the type of AMQPExchange must be direct | fanout | topic" );
	}
}


void AMQPExchange::Delete() {
	if (name.empty())
		throw AMQPException("the name of exchange not set");

	sendDeleteCommand();
}

void AMQPExchange::Delete(string name) {
	this->name = name;
	sendDeleteCommand();
}

void AMQPExchange::sendDeleteCommand(){
	amqp_bytes_t exchange = amqp_cstring_bytes(name.c_str());
	amqp_exchange_delete_t s;
	{
		s.ticket 		= 0;
		s.exchange 	= exchange;
		s.if_unused = ( AMQP_IFUNUSED & params ) 	? 1:0;
		s.nowait 		= ( AMQP_NOWAIT & params )	 	? 1:0;
	}

	amqp_method_number_t method_ok = AMQP_EXCHANGE_DELETE_OK_METHOD;

	amqp_rpc_reply_t res = amqp_simple_rpc(
		*cnn,
		channelNum,
		AMQP_EXCHANGE_DELETE_METHOD,
		&method_ok,
		&s
	);

	AMQPBase::checkReply(&res);
}

void AMQPExchange::Bind(string name) {
	if (type != ExchangeType::FANOUT) {
		throw AMQPException("key is NULL, this using only for the type fanout" );
	}

	sendBindCommand(name.c_str(), NULL );
}

void AMQPExchange::Bind(string name, string key) {
	sendBindCommand(name.c_str(), key.c_str());
}

void AMQPExchange::sendBindCommand(const char * queue, const char * key) {
	amqp_bytes_t queueByte = amqp_cstring_bytes(queue);
	amqp_bytes_t exchangeByte = amqp_cstring_bytes(name.c_str());
	amqp_bytes_t keyByte = amqp_cstring_bytes(key);

  amqp_queue_bind_t s;
  {
  	s.ticket 								= 0;
		s.queue 								= queueByte;
		s.exchange 							= exchangeByte;
		s.routing_key 					= keyByte;
		s.nowait 								= ( AMQP_NOWAIT & params ) ? 1:0;

		s.arguments.num_entries = 0;
		s.arguments.entries 		= NULL;
	}

	amqp_method_number_t method_ok = AMQP_QUEUE_BIND_OK_METHOD;
    amqp_rpc_reply_t res = amqp_simple_rpc(
		*cnn,
		channelNum,
		AMQP_QUEUE_BIND_METHOD,
		&method_ok,
		&s
	);

	AMQPBase::checkReply(&res);
}

void AMQPExchange::Publish(string message, string key) {
	amqp_bytes_t messageByte = amqp_bytes_malloc(message.length());
	memcpy(messageByte.bytes, message.c_str(), message.length());
	sendPublishCommand(messageByte, key.c_str());
}

void AMQPExchange::Publish(const char * data, uint32_t length, string key) {
	amqp_bytes_t messageByte = amqp_bytes_malloc(length);
	memcpy(messageByte.bytes, data, length);
	sendPublishCommand(messageByte, key.c_str());
}

void AMQPExchange::sendPublishCommand(amqp_bytes_t messageByte, const char * key) {
	amqp_bytes_t exchangeByte = amqp_cstring_bytes(name.c_str());
	amqp_bytes_t keyrouteByte = amqp_cstring_bytes(key);

	amqp_basic_properties_t props;

	if (sHeaders.find("Content-type")!= sHeaders.end())
		props.content_type = amqp_cstring_bytes(sHeaders["Content-type"].c_str());
	else
		props.content_type = amqp_cstring_bytes("text/plain");

	if (iHeaders.find("Delivery-mode")!= iHeaders.end())
		props.delivery_mode = (uint8_t)iHeaders["Delivery-mode"];
	else
		props.delivery_mode = 2; // Persistent delivery mode

	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;

	if (sHeaders.find("Content-encoding")!= sHeaders.end()) {
		props.content_encoding = amqp_cstring_bytes(sHeaders["Content-encoding"].c_str());
		props._flags += AMQP_BASIC_CONTENT_ENCODING_FLAG;
	}

	if (sHeaders.find("message_id")!= sHeaders.end()) {
		props.message_id = amqp_cstring_bytes(sHeaders["message_id"].c_str());
		props._flags += AMQP_BASIC_MESSAGE_ID_FLAG;
	}

	if (sHeaders.find("user_id")!= sHeaders.end()) {
		props.user_id = amqp_cstring_bytes(sHeaders["user_id"].c_str());
		props._flags += AMQP_BASIC_USER_ID_FLAG;
	}

	if (sHeaders.find("app_id")!= sHeaders.end()) {
		props.app_id = amqp_cstring_bytes(sHeaders["app_id"].c_str());
		props._flags += AMQP_BASIC_APP_ID_FLAG;
	}

	if (sHeaders.find("cluster_id")!= sHeaders.end()) {
		props.cluster_id = amqp_cstring_bytes(sHeaders["cluster_id"].c_str());
		props._flags += AMQP_BASIC_CLUSTER_ID_FLAG;
	}

	if (sHeaders.find("correlation_id")!= sHeaders.end()) {
		props.correlation_id = amqp_cstring_bytes(sHeaders["correlation_id"].c_str());
		props._flags += AMQP_BASIC_CORRELATION_ID_FLAG;
	}

	if (iHeaders.find("priority")!= iHeaders.end()) {
		props.priority = (uint8_t) iHeaders["priority"];
		props._flags += AMQP_BASIC_PRIORITY_FLAG;
	}

	if (iHeaders.find("timestamp")!= iHeaders.end()) {
		props.timestamp = (uint64_t) iHeaders["timestamp"];
		props._flags += AMQP_BASIC_TIMESTAMP_FLAG;
	}

	if (sHeaders.find("Expiration")!= sHeaders.end()) {
		props.expiration = amqp_cstring_bytes(sHeaders["Expiration"].c_str());
		props._flags += AMQP_BASIC_EXPIRATION_FLAG;
	}

	if (sHeaders.find("type")!= sHeaders.end()) {
		props.type = amqp_cstring_bytes(sHeaders["type"].c_str());
		props._flags += AMQP_BASIC_TYPE_FLAG;
	}

	if (sHeaders.find("Reply-to")!= sHeaders.end()) {
		props.reply_to = amqp_cstring_bytes(sHeaders["Reply-to"].c_str());
		props._flags += AMQP_BASIC_REPLY_TO_FLAG;
	}

	props.headers.num_entries = sHeadersSpecial.size();
	amqp_table_entry_t_ entries[props.headers.num_entries];

	int i = 0;
	map<string, string>::iterator it;
	for (it = sHeadersSpecial.begin(); it != sHeadersSpecial.end(); it++) {
		entries[i].key = amqp_cstring_bytes((*it).first.c_str());
		entries[i].value.kind = AMQP_FIELD_KIND_UTF8;
		entries[i].value.value.bytes = amqp_cstring_bytes((*it).second.c_str());
		i++;
	}
	props.headers.entries = entries;
	props._flags += AMQP_BASIC_HEADERS_FLAG;

	short mandatory = (params & AMQP_MANDATORY) ? 1:0;
	short immediate = (params & AMQP_IMMIDIATE) ? 1:0;

	int res = amqp_basic_publish(
		*cnn,
		1,
		exchangeByte,
		keyrouteByte,
		mandatory,
		immediate,
		&props,
		messageByte
	);

	if ( 0 > res ) {
		throw AMQPException("AMQP Publish Failed");
	}
}

void AMQPExchange::setHeader(string name, int value) {
	iHeaders[name] = value;
}

void AMQPExchange::setHeader(string name, string value, bool special) {
	if (special) {
		sHeadersSpecial[name] = value;
	} else {
		sHeaders[name] = value;
	}
}

void AMQPExchange::setHeader(string name, string value) {
	setHeader(name, value, 0);
}
