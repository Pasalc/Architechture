#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstdio>
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include <Poco/Data/MySQL/MySQLException.h>
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/OptionSet.h"
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/MySQL/Connector.h>
#include "Poco/Exception.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/HTMLForm.h"

using namespace std;

#define POCO_CHECKER(...)                        \
try{                                             \
    __VA_ARGS__                                  \
}                                                \
catch(Poco::Data::MySQL::StatementException &e){ \
    cout<<"statement ERROR:"<<e.what()<<"\n";    \
}                                                \
catch(Poco::Data::MySQL::ConnectionException &e){\
    cout<<"connection ERROR:"<<e.what()<<"\n";   \
}

struct Person{   
    string login;
    string first_name;
    string last_name;
    int age;
};

const string HOST="localhost";
const string IP="172.17.0.1";//TODO
const string LOGIN="stud";
const string DATABASE="Age";//TODO
const string PASSWORD="stud";
const int PORT=8080;

bool Is_Prefix(const string&, const string&);

Poco::Data::Session *Create_Session(){ 
    string connection_string="host="+HOST+";user="+LOGIN+";db="+DATABASE+";password="+PASSWORD;
    Poco::Data::MySQL::Connector::registerConnector();
    Poco::Data::Session *createsession=NULL;
    POCO_CHECKER(createsession=new Poco::Data::Session(Poco::Data::SessionFactory::instance().create(Poco::Data::MySQL::Connector::KEY,connection_string));)
    return createsession;
}