#include "amqp11.hpp"

using std::string;


AMQPBase::~AMQPBase() {
	this->closeChannel();
}

std::string AMQPBase::randomStr(size_t length) {
	auto randChar = []() -> char {
		const char charset[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[ rand() % max_index ];
	};
	std::string str(length, 0);
	std::generate_n( str.begin(), length, randChar );

	std::stringstream ss;
	ss << "queue.gen-" << str;
	return ss.str();
}

void AMQPBase::checkReply(amqp_rpc_reply_t * res) {
	checkClosed(res);
	if (res->reply_type != AMQP_RESPONSE_NORMAL ) {
		throw AMQPException(res);
	}
}

void AMQPBase::checkClosed(amqp_rpc_reply_t * res)
{
	if( (res->reply_type == AMQP_RESPONSE_SERVER_EXCEPTION)
							&& (res->reply.id == AMQP_CHANNEL_CLOSE_METHOD) )
	{
		this->opened = 0;
	}
}

void AMQPBase::openChannel() {
	amqp_channel_open(*cnn, channelNum);
	amqp_rpc_reply_t res = amqp_get_rpc_reply(*cnn);

	if ( res.reply_type != AMQP_RESPONSE_NORMAL) {
		throw AMQPException(&res);
	}
	this->opened = 1;
}

void AMQPBase::closeChannel() {
	if (opened)
		amqp_channel_close(*cnn, channelNum, AMQP_REPLY_SUCCESS);
}

void AMQPBase::reopen() {
	if (opened) return;
	AMQPBase::openChannel();
}

int AMQPBase::getChannelNum() {
	return channelNum;
}

void AMQPBase::setParam(short param) {
	this->params = param;
}

string AMQPBase::getName() {
	return this->name;
}

void AMQPBase::setName(string name) {
	this->name = name;
}
