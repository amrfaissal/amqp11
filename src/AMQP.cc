#include "amqp++.h"

AMQP::AMQP() {
	AMQP::init();
	AMQP::initDefault();
	AMQP::connect();
};

AMQP::AMQP(std::string cnnStr) {
	AMQP::init();
	AMQP::parseCnnString(cnnStr);
	AMQP::connect();
};

AMQP::~AMQP() {
	if (channels.size()) {
		std::vector<AMQPBase*>::iterator i;
		for (i=channels.begin(); i!=channels.end(); i++) {
			delete *i;
		}
	}

	amqp_destroy_connection(cnn);
	close(sockfd);
};

void AMQP::init() {
	exchange=NULL;
	channelNumber=0;
}

void AMQP::initDefault() {
	host = std::string(AMQPHOST);
	port = AMQPPORT;
	vhost = std::string(AMQPVHOST);
	user = std::string(AMQPLOGIN);
	password = std::string(AMQPPSWD);
}

void AMQP::parseCnnString( std::string cnnString ) {
	 if (!cnnString.size()) {
		AMQP::initDefault();
		return;
	 }

	// find '@' if Ok -> right part is host:port else all host:port
	 std::string hostPortStr, userPswString;
	int pos = cnnString.find('@');

	switch (pos) {
		case 0:
			hostPortStr.assign(cnnString, 1, cnnString.size()-1);
			AMQP::parseHostPort(hostPortStr);
			user = std::string(AMQPLOGIN);
			password = std::string(AMQPPSWD);
		break;
		case -1:
			AMQP::parseHostPort(cnnString);
			user = std::string(AMQPLOGIN);
			password = std::string(AMQPPSWD);
		break;
		default :
			hostPortStr.assign(cnnString, pos+1, cnnString.size()-pos+1);
			userPswString.assign(cnnString, 0, pos);
			AMQP::parseHostPort(hostPortStr);
			AMQP::parseUserStr(userPswString );
		break;
	}
}

void AMQP::parseUserStr(std::string userString) {
	int pos = userString.find(':');
	switch (pos) {
		case 0:
			user.assign(userString, 1, userString.size()-1);
			password=AMQPPSWD;
		break;
		case -1:
			user=userString;
			password=AMQPPSWD;
		break;
		default:
			user.assign(userString, pos+1, userString.size()+1-pos);
			password.assign(userString, 0, pos);
		break;
	}
}

void AMQP::parseHostPort(std::string hostPortString ) {
	size_t pos = hostPortString.find(':');
	std::string hostString;
	std::string portString;

	size_t pos2 = hostPortString.find('/');

        host  = AMQPHOST;
        vhost = AMQPVHOST;
        port  = AMQPPORT;

        if (pos == std::string::npos) {
                if ( pos2 == std::string::npos) {
                        host = hostPortString;
                } else {
                        vhost.assign(hostPortString, pos2+1, hostPortString.size()-pos2);
                        if (pos2 != 0) {
                                host.assign(hostPortString, 0, pos2);
                        }
                }
        } else if (pos == 0) {
                if (pos2 == std::string::npos) {
                        portString.assign(hostPortString, 1, hostPortString.size()-1);
                } else {
                        portString.assign(hostPortString, 1, pos2-1);
                        vhost.assign(hostPortString, pos2+1, hostPortString.size()-pos2);
                }
                port = atoi(portString.c_str());
        } else {
                if ( pos2 == std::string::npos ) {
                        host.assign(hostPortString, 0, pos);
                        portString.assign(hostPortString, pos+1, hostPortString.size()-pos+1);
                } else {
                        vhost.assign(hostPortString, pos2+1, hostPortString.size()-pos2);
                        host.assign(hostPortString, 0, pos);
                        portString.assign(hostPortString, pos+1, pos2-pos-1);
                }
                port = atoi(portString.c_str());
        }
}

void AMQP::connect() {
	AMQP::sockConnect();
	AMQP::login();
}

void AMQP::printConnect() {
	std::cout << "AMQP connection: \n";

	std::cout << "port  = " << port << "\n";
	std::cout << "host  = " << host << "\n";
	std::cout << "vhost = " << vhost << "\n";
	std::cout << "user  = " << user << "\n";
	std::cout << "passw = " << password << "\n";
}

void AMQP::sockConnect() {
	cnn = amqp_new_connection();
	sockfd = amqp_open_socket(host.c_str(), port);

	if (sockfd < 0){
		amqp_destroy_connection(cnn);
		throw AMQPException("AMQP cannot create socket descriptor");
	}

	//cout << "sockfd="<< sockfd  << "  pid=" <<  getpid() <<endl;
	amqp_set_sockfd(cnn, sockfd);
}

void AMQP::login() {
	amqp_rpc_reply_t res = amqp_login(cnn, vhost.c_str(), 0, FRAME_MAX, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), password.c_str());
	if ( res.reply_type == AMQP_RESPONSE_NORMAL)
		return;

	amqp_destroy_connection(cnn);
	throw AMQPException(&res);
}

AMQPExchange * AMQP::createExchange() {
	channelNumber++;
	AMQPExchange * exchange = new AMQPExchange(&cnn,channelNumber);
	channels.push_back( dynamic_cast<AMQPBase*>(exchange) );
	return exchange;
}

AMQPExchange * AMQP::createExchange(std::string name) {
	channelNumber++;
	AMQPExchange * exchange = new AMQPExchange(&cnn,channelNumber,name);
	channels.push_back( dynamic_cast<AMQPBase*>(exchange) );
	return exchange;
}

AMQPQueue * AMQP::createQueue() {
	channelNumber++;
	AMQPQueue * queue = new AMQPQueue(&cnn,channelNumber);
	channels.push_back( dynamic_cast<AMQPBase*>(queue) );
	return queue;
}

AMQPQueue * AMQP::createQueue(std::string name) {
        channelNumber++;
	AMQPQueue * queue = new AMQPQueue(&cnn,channelNumber,name);
	channels.push_back( dynamic_cast<AMQPBase*>(queue) );
	return queue;
}

void AMQP::closeChannel() {
	channelNumber--;
	AMQPBase * cnn = channels.back();
	if (cnn) {
		delete cnn;
		channels.pop_back();
	}
}
