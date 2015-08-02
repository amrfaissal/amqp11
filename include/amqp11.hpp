/**
 *	amqp11.hpp:
 * 				amqp11 is a small and simple AMQP client library for C++11
 *
 */
#pragma once

#define AMQPPORT 5672
#define AMQPHOST "localhost"
#define AMQPVHOST "/"
#define AMQPLOGIN "guest"
#define AMQPPSWD  "guest"

#define AMQPDEBUG ":5673"

#define AMQP_AUTODELETE		1
#define AMQP_DURABLE		2
#define AMQP_PASSIVE		4
#define AMQP_MANDATORY		8
#define AMQP_IMMIDIATE		16
#define AMQP_IFUNUSED		32
#define AMQP_EXCLUSIVE		64
#define AMQP_NOWAIT			128
#define AMQP_NOACK			256
#define AMQP_NOLOCAL		512
#define AMQP_MULTIPLE		1024

#define HEADER_FOOTER_SIZE 8 //  7 bytes up front, then payload, then 1 byte footer
#define FRAME_MAX 131072    // max lenght (size) of frame

#define RAND_STR_LENGTH 8

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <strings.h>

#include <unistd.h>
#include <stdint.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>


class AMQPQueue;

enum class AMQPEventType {
	AMQP_MESSAGE, AMQP_SIGUSR, AMQP_CANCEL, AMQP_CLOSE_CHANNEL
};

class AMQPException {
	std::string message;
	int code;
public:
	AMQPException(std::string message);
	AMQPException(amqp_rpc_reply_t * res);

	std::string getMessage();
	uint16_t getReplyCode();
};

class AMQPMessage {
	char * data;
	uint32_t len;
	std::string exchange;
	std::string routing_key;
	uint32_t delivery_tag;
	int message_count;
	std::string consumer_tag;
	AMQPQueue * queue;
	std::map<std::string, std::string> headers;

public:
	AMQPMessage(AMQPQueue * queue);
	~AMQPMessage();

	void setMessage(const char * data, uint32_t length);
	std::string getMessage(uint32_t * length);

	void addHeader(std::string name, amqp_bytes_t * value);
	void addHeader(std::string name, uint64_t * value);
	void addHeader(std::string name, uint8_t * value);
	void addHeader(amqp_bytes_t * name, amqp_bytes_t * value);
	std::string getHeader(std::string name);

	void setConsumerTag(amqp_bytes_t consumer_tag);
	void setConsumerTag(std::string consumer_tag);
	std::string getConsumerTag();

	void setMessageCount(int count);
	int getMessageCount();

	void setExchange(amqp_bytes_t exchange);
	void setExchange(std::string exchange);
	std::string getExchange();

	void setRoutingKey(amqp_bytes_t routing_key);
	void setRoutingKey(std::string routing_key);
	std::string getRoutingKey();

	uint32_t getDeliveryTag();
	void setDeliveryTag(uint32_t delivery_tag);

	AMQPQueue * getQueue();
};

class AMQPBase
{
protected:
	std::string name;
	short params;
	amqp_connection_state_t * cnn;
	int channelNum;
	AMQPMessage * pmessage;

	short opened;

	void checkReply(amqp_rpc_reply_t * res);
	void checkClosed(amqp_rpc_reply_t * res);
	void openChannel();

	std::string randomStr(size_t length);

public:
	virtual ~AMQPBase();
	int getChannelNum();
	void setParam(short param);
	std::string getName();
	void closeChannel();
	void reopen();
	void setName(const char * name);
	void setName(std::string name);
};

class AMQPQueue: public AMQPBase
{
protected:
	std::map<AMQPEventType, std::function<int(AMQPMessage*)>> events;
	amqp_bytes_t consumer_tag;
	uint32_t delivery_tag;
	uint32_t count;

public:
	AMQPQueue(amqp_connection_state_t * cnn, int channelNum);

	void Declare();
	void Declare(std::string name);
	void Declare(std::string name, short params);
	void Declare(short params);

	void Delete();
	void Delete(std::string name);

	void Purge();
	void Purge(std::string name);

	void Bind(std::string exchangeName, std::string key);

	void unBind(std::string exchangeName, std::string key);

	void Get();
	void Get(short param);

	void Consume();
	void Consume(short param);

	void Cancel(amqp_bytes_t consumer_tag);
	void Cancel(std::string consumer_tag);

	void Ack();
	void Ack(uint32_t delivery_tag);

	AMQPMessage * getMessage()
	{
		return pmessage;
	}

	uint32_t getCount()
	{
		return count;
	}

	void setConsumerTag(std::string consumer_tag);
	amqp_bytes_t getConsumerTag();

	void addEvent(AMQPEventType eventType, std::function<int(AMQPMessage*)> callable);

	virtual ~AMQPQueue();

	void Qos(uint32_t prefetch_size, uint16_t prefetch_count,
			amqp_boolean_t global);

private:
	void sendDeclareCommand();
	void sendDeleteCommand();
	void sendPurgeCommand();
	void sendBindCommand(const char * exchange, const char * key);
	void sendUnBindCommand(const char * exchange, const char * key);
	void sendGetCommand();
	void sendConsumeCommand();
	void sendCancelCommand();
	void sendAckCommand();
	void setHeaders(amqp_basic_properties_t * p);
};

enum class ExchangeType
{
	DIRECT, FANOUT, TOPIC
};

class AMQPExchange: public AMQPBase
{
	ExchangeType type;
	std::map<std::string, std::string> sHeaders;
	std::map<std::string, std::string> sHeadersSpecial;
	std::map<std::string, int> iHeaders;

public:
	AMQPExchange(amqp_connection_state_t * cnn, int channelNum);
	virtual ~AMQPExchange();

	void Declare(std::string name);
	void Declare(std::string name, ExchangeType type);
	void Declare(std::string name, ExchangeType type, short params);

	void Delete();
	void Delete(std::string name);

	void Bind(std::string queueName);
	void Bind(std::string queueName, std::string key);

	void Publish(std::string message, std::string key);
	void Publish(const char * data, uint32_t length, std::string key);

	void setHeader(std::string name, int value);
	void setHeader(std::string name, std::string value);
	void setHeader(std::string name, std::string value, bool special);

private:
	AMQPExchange();
	void checkType();
	const char * exchangetype_ctr(ExchangeType type);

	void sendDeclareCommand();
	void sendDeleteCommand();
	void sendPublishCommand();

	void sendBindCommand(const char * queueName, const char * key);
	void sendPublishCommand(amqp_bytes_t messageByte, const char * key);
	void sendCommand();
	void checkReply(amqp_rpc_reply_t * res);
	void checkClosed(amqp_rpc_reply_t * res);
};

class AMQP
{
	int port;
	std::string host;
	std::string vhost;
	std::string user;
	std::string password;

	int channelNumber;

	boost::shared_ptr<AMQPExchange> exchange;
	std::vector<boost::shared_ptr<AMQPBase>> channels;

	amqp_socket_t * socket;
	amqp_connection_state_t conn;

	struct timeval * tv;

public:
	AMQP();
	AMQP(std::string connStr);
	~AMQP();

	boost::shared_ptr<AMQPExchange>	createExchange();
	boost::shared_ptr<AMQPQueue>		createQueue();

	void printConnect();

	void closeChannel();

private:
	AMQP(AMQP &obj);
	void init();
	void initDefault();
	void connect();
	void parseCnnString(std::string cnnString);
	void parseHostPort(std::string hostPortString);
	void parseUserStr(std::string userString);
	void sockConnect();
	void login();
};

// utility functions
class fn {
	static inline std::string & ltrim(std::string & s) {
		s.erase(
			s.begin(),
			std::find_if(s.begin(), s.end(),
									std::not1(std::ptr_fun<int,int>(std::isspace))));
		return s;
	}
	static inline std::string & rtrim(std::string & s) {
		s.erase(
			std::find_if(s.rbegin(), s.rend(),
							std::not1(std::ptr_fun<int,int>(std::isspace))).base(),
			s.end()
		);
		return s;
	}
public:
	static inline std::string & trim(std::string & s) {
		return ltrim(rtrim(s));
	}
};
