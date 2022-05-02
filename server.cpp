#include "GlobData.hpp"

class HTML_Receiver : public Poco::Net::HTTPRequestHandler{
public:
    HTML_Receiver(const string &http_format) : _http_format(http_format){
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response){
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");
        ostream &ost=response.send();
        string url=request.getURI();
        string html_place="html"+url;
        FILE *html=fopen(html_place.c_str(),"rb");
        if(!html){
            cout<<html_place<<" not found\n";
            ost<<"<html lang=\"ru\"><head><title>Web Server</title></head><body><h1>404 moment</h1></body></html>\n";
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        }
        else{
            vector<char> chars(400);
            int read;
            while(read=fread(chars.data(),sizeof(char),chars.size()-1,html)){
                chars[read]='\0';
                ost<<chars.data();
            }
            fclose(html);
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        }
    }

private:
    string _http_format;
};

class Database_Receiver : public Poco::Net::HTTPRequestHandler{
public:
    Database_Receiver(const string &http_format) : _http_format(http_format){
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response){
        Poco::Net::HTMLForm request_form(request,request.stream());
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        ostream &ost=response.send();
        string str_request=request.getMethod();
        if(str_request=="GET"&&request_form.has("login")){
            string form_login=request_form.get("login"); 
            auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
            auto &session=*createsession;
            Person human;
            bool found=true;
            POCO_CHECKER(
                Poco::Data::Statement database_request(session);
                database_request<<"SELECT login, first_name, last_name, age FROM Person WHERE login=? -- sharding:"<<to_string(hash<string>{}(form_login)%DATABASE_SHARDS),Poco::Data::Keywords::into(human.login),Poco::Data::Keywords::into(human.first_name),Poco::Data::Keywords::into(human.last_name),Poco::Data::Keywords::into(human.age),Poco::Data::Keywords::use(form_login),Poco::Data::Keywords::range(0, 1);
                database_request.execute();
                Poco::Data::RecordSet record_set(database_request);
                if(!record_set.moveFirst()){
                    throw logic_error("not found");
                } 
            )
            catch(logic_error &e){
                cout<<form_login<<" not found\n";
                found=false;
            }
            try{
                Poco::JSON::Array result_json;
                Poco::JSON::Object::Ptr object_json = new Poco::JSON::Object();
                if(found){
                    object_json->set("login", human.login);
                    object_json->set("first_name", human.first_name);
                    object_json->set("last_name", human.last_name);
                    object_json->set("age", human.age);
                }
                result_json.add(object_json);
                Poco::JSON::Stringifier::stringify(result_json, ost);
            }
            catch(...){
                cout<<"exception\n";
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        }
        else if(str_request=="GET"&&request_form.has("first_name")&&request_form.has("last_name")){
            string first_name=request_form.get("first_name"),last_name=request_form.get("last_name");
            auto Parallel=[](int shard_part,string first_name,string last_name,vector<Person> *database_result)->void{
                auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
                Poco::Data::Session &session=*createsession;
                Person human;
                POCO_CHECKER(
                    Poco::Data::Statement database_request(session);
                    database_request<<"SELECT login, first_name, last_name, age FROM Person WHERE first_name LIKE ? AND last_name LIKE ? -- sharding:"<<to_string(shard_part),Poco::Data::Keywords::into(human.login),Poco::Data::Keywords::into(human.first_name),Poco::Data::Keywords::into(human.last_name),Poco::Data::Keywords::into(human.age),Poco::Data::Keywords::use(first_name),Poco::Data::Keywords::use(last_name),Poco::Data::Keywords::range(0, 1);
                    while(!database_request.done()){
                        if(database_request.execute()){
                            database_result->push_back(human);
                        }
                    }
                )
            };
            vector<vector<Person>*> result(DATABASE_SHARDS);
            vector<thread*> threads(DATABASE_SHARDS);
            for(int i=0;i<DATABASE_SHARDS;i++){
                result[i] = new vector<Person>(0);
                threads[i] = new thread(Parallel,i, first_name,last_name,result[i]);
            }
            for(int i=0;i<threads.size();i++){
                threads[i]->join(); 
                delete threads[i];
            }
            try{
                Poco::JSON::Array result_json;
                for(int i=0;i<result.size();i++) 
                    for(int j=0;j<result[i]->size();j++){
                        Poco::JSON::Object::Ptr object_json=new Poco::JSON::Object();
                        object_json->set("login",result[i]->at(j).login);
                        object_json->set("first_name",result[i]->at(j).first_name);
                        object_json->set("last_name",result[i]->at(j).last_name);
                        object_json->set("age",result[i]->at(j).age);
                        result_json.add(object_json);
                    }
                Poco::JSON::Stringifier::stringify(result_json,ost);
            }
            catch(...){
                cout<<"exception\n";
            }
            for(int i=0;i<result.size();i++)
                delete result[i];
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        }
        else if(str_request=="POST"&&request_form.has("login")&&request_form.has("last_name")&&request_form.has("first_name")&&request_form.has("age")){
            string login=request_form.get("login"),last_name=request_form.get("last_name"),first_name=request_form.get("first_name");
            int age=atoi(request_form.get("age").c_str());
            auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
            auto &session=*createsession;
            bool found=true;
            POCO_CHECKER(
                Poco::Data::Statement database_request(session);
                database_request<<"SELECT login FROM Person WHERE login=? -- sharding:"<<to_string(hash<string>{}(login)%DATABASE_SHARDS),Poco::Data::Keywords::use(login),Poco::Data::Keywords::range(0, 1);
                database_request.execute();
                Poco::Data::RecordSet record_set(database_request);
                if(record_set.moveFirst()) {
                    found = false;
                }
            )
            string res;
            if(found){
                res="OK";
                POCO_CHECKER(
                    Poco::Data::Statement INSERT(session);
                    INSERT<<"INSERT INTO Person (login, first_name, last_name, age)VALUES (?, ?, ?, ?) -- sharding:"<<to_string(hash<string>{}(login)%DATABASE_SHARDS),Poco::Data::Keywords::use(login),Poco::Data::Keywords::use(first_name),Poco::Data::Keywords::use(last_name),Poco::Data::Keywords::use(age),Poco::Data::Keywords::range(0, 1);
                    INSERT.execute();
                )
            }
            else{
                res="Already exists";
            }
            ost<<"<html lang=\"ru\"><head><title>Web Server</title></head><body><h1>"+res+"</h1></body></html>";
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        }
        else{
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        }
    }

private:
    string _http_format;
};

class Receiver_Chooser : public Poco::Net::HTTPRequestHandlerFactory{
public:
    Receiver_Chooser(const string &http_format) : _http_format(http_format){
    }

    Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request){
        if(Is_Prefix(request.getURI(), "/person")){
            return new Database_Receiver(_http_format);
        }
        else{
            return new HTML_Receiver(_http_format);
        }
    }

private:
    string _http_format;
};

class Server_App : public Poco::Util::ServerApplication{
public:
    Server_App() : _helpRequested(false){
    }

    ~Server_App(){
    }

protected:
    void Create_Server(Poco::Util::Application &self){
        loadConfiguration();
        Poco::Util::ServerApplication::initialize(self);
    }

    void Delete_Server(){
        Poco::Util::ServerApplication::uninitialize();
    }

    void Options_Server(Poco::Util::OptionSet &options){
        Poco::Util::ServerApplication::defineOptions(options);
    }

    int main(const vector<string> &args){
        if(!_helpRequested){
            unsigned short port=(unsigned short)config().getInt("Server_App.port",PORT);
            string http_format(config().getString("Server_App.http_format",Poco::DateTimeFormat::SORTABLE_FORMAT));
            Poco::Net::ServerSocket server_socket(Poco::Net::SocketAddress("0.0.0.0",port));
            Poco::Net::HTTPServer server(new Receiver_Chooser(http_format),server_socket,new Poco::Net::HTTPServerParams);
            cout<<"Started server on "<<IP<<":"<<to_string(port)<<"\n";
            server.start();
            waitForTerminationRequest();
            server.stop();
        }
        return Poco::Util::Application::EXIT_OK;
    }

private:
    bool _helpRequested;
};

bool Is_Prefix(const string &first, const string &second){
    int size = second.size();
    if(size>first.size()){
        return false;
    }
    for(int i=0;i<size;i++){
        if(first[i]!=second[i]){
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]){
    cout<<"Enter ip: ";
    cin>>IP;
    Server_App app;
    return app.run(1, argv);
}