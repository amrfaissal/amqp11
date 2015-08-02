#include "amqp11.hpp"

AMQP::AMQP() {
	AMQP::init();
	AMQP::initDefault();
	AMQP::connect();
};

AMQP::AMQP(std::string connStr) {
	AMQP::init();
	AMQP::parseCnnString(connStr);
	AMQP::connect();
};

AMQP::~AMQP() {
	if (channels.size()) {
		std::vector<boost::shared_ptr<AMQPBase>>::iterator it;
		for ( it = channels.begin(); it != channels.end(); ++it) {
      (*it).reset();
		}
	}
	amqp_destroy_connection(conn);
};

void AMQP::init() {
	exchange = nullptr;
	channelNumber = 0;
}

void AMQP::initDefault() {
	this->host     = std::string(AMQPHOST);
	this->port     = AMQPPORT;
	this->vhost    = std::string(AMQPVHOST);
	this->user     = std::string(AMQPLOGIN);
	this->password = std::string(AMQPPSWD);
}

void AMQP::parseCnnString( std::string cnnString ) {
	 if (cnnString.empty()) {
		AMQP::initDefault();
		return;
	 }

   std::string hostPortStr, userPswString;

   // find '@' if Ok -> right part is host:port else all host:port
   int pos = cnnString.find('@');
   switch (pos) {
    case 0:
    {
      hostPortStr.assign(cnnString, 1, cnnString.size()-1);
      AMQP::parseHostPort(hostPortStr);

      this->user     = std::string(AMQPLOGIN);
      this->password = std::string(AMQPPSWD);
    } break;

    case -1:
    {
      AMQP::parseHostPort(cnnString);

      this->user     = std::string(AMQPLOGIN);
			this->password = std::string(AMQPPSWD);
    } break;

    default :
    {
			hostPortStr.assign(cnnString, pos+1, cnnString.size()-pos+1);
			userPswString.assign(cnnString, 0, pos);

      AMQP::parseHostPort(hostPortStr);
			AMQP::parseUserStr(userPswString );
		}
  }
}

void AMQP::parseUserStr(std::string userString) {
	int pos = userString.find(':');
	switch (pos) {
		case 0:
    {
			user.assign(userString, 1, userString.size()-1);
			this->password = AMQPPSWD;
    } break;

		case -1:
    {
			this->user     = userString;
			this->password = AMQPPSWD;
    } break;

		default:
    {
      user.assign(userString, pos + 1, userString.size() + 1 - pos);
      password.assign(userString, 0, pos);
    }
	}
}

void AMQP::parseHostPort(std::string hostPortString ) {
	size_t pos = hostPortString.find(':');
	std::string hostString;
	std::string portString;

	size_t pos2 = hostPortString.find('/');
  {
    this->host  = AMQPHOST;
    this->vhost = AMQPVHOST;
    this->port  = AMQPPORT;
  }

  if (pos == std::string::npos) {
    if ( pos2 == std::string::npos) {
      host = hostPortString;
    } else {
      vhost.assign(hostPortString, pos2 + 1, hostPortString.size() - pos2);
      if (pos2 != 0) {
        host.assign(hostPortString, 0, pos2);
      }
    }
  } else if (pos == 0) {
    if (pos2 == std::string::npos) {
      portString.assign(hostPortString, 1, hostPortString.size() - 1);
    } else {
      portString.assign(hostPortString, 1, pos2 - 1);
      vhost.assign(hostPortString, pos2 + 1, hostPortString.size() - pos2);
    }

    port = boost::lexical_cast<int>(portString);
  } else {
    if ( pos2 == std::string::npos ) {
      host.assign(hostPortString, 0, pos);
      portString.assign(hostPortString, pos+1, hostPortString.size() - pos + 1);
    } else {
      vhost.assign(hostPortString, pos2 + 1, hostPortString.size() - pos2);
      host.assign(hostPortString, 0, pos);
      portString.assign(hostPortString, pos+1, pos2-pos-1);
    }

    port = boost::lexical_cast<int>(portString);
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
	this->conn = amqp_new_connection();
  this->socket = amqp_tcp_socket_new(conn);

	if (!socket){
		amqp_destroy_connection(conn);
		throw AMQPException("AMQP cannot create tcp socket");
	}

  int status = amqp_socket_open(socket, host.c_str(), port);
  if (status) {
    amqp_destroy_connection(conn);
    throw AMQPException("AMQP could not open the socket");
  }
}

void AMQP::login() {
	amqp_rpc_reply_t res = amqp_login(conn, vhost.c_str(), 0, FRAME_MAX, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), password.c_str());
	if ( res.reply_type == AMQP_RESPONSE_NORMAL) {
		return;
  }
	amqp_destroy_connection(conn);
	throw AMQPException(&res);
}

boost::shared_ptr<AMQPExchange> AMQP::createExchange() {
	channelNumber++;
	boost::shared_ptr<AMQPExchange> exchange(new AMQPExchange(&conn, channelNumber));
	channels.push_back( boost::dynamic_pointer_cast<AMQPBase>(exchange) );
	return exchange;
}

boost::shared_ptr<AMQPQueue> AMQP::createQueue() {
	channelNumber++;
	boost::shared_ptr<AMQPQueue> queue(new AMQPQueue(&conn, channelNumber));
	channels.push_back( boost::dynamic_pointer_cast<AMQPBase>(queue) );
	return queue;
}

void AMQP::closeChannel() {
	channelNumber--;
	boost::shared_ptr<AMQPBase> cnn = channels.back();
	if (cnn) {
		cnn.reset();
		channels.pop_back();
	}
}
