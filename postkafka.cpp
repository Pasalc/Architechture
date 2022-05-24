#include "GlobData.hpp"

int main(int argc, char *argv[]){
    try{
        signal(SIGINT,[](int){KAFKA_WORK=false;});
        cppkafka::Configuration data_config={{"metadata.broker.list",QUEUE},{"group.id",to_string(KAFKA_GROUP)},{"enable.auto.commit",false}};
        cppkafka::Consumer data_consumer(data_config);
        data_consumer.set_assignment_callback([](const cppkafka::TopicPartitionList &kafka_group){cout<<"Got assigned: "<<kafka_group<<"\n";});
        data_consumer.set_revocation_callback([](const cppkafka::TopicPartitionList &kafka_group){cout<<"Got revoked: "<<kafka_group<<"\n";});
        data_consumer.subscribe({TOPIC});
        cout<<"Getting messages from topic "<<TOPIC<<"\n";
        while(KAFKA_WORK){
            cppkafka::Message message=data_consumer.poll();
            if(message){
                if(message.get_error()&&!message.is_eof()){
                    cout<<"Got error: "<<message.get_error()<<"\n";
                }
                else{
                    if(message.get_key()){
                        cout<<message.get_key()<<" goes to ";
                    }
                    cout<<message.get_payload()<<"\n";
                    Poco::JSON::Parser tmp;
                    Person human;
                    bool found=true;
                    string message_load=message.get_payload();
                    try{
                        Poco::Dynamic::Var parser=tmp.parse(message_load);
                        Poco::JSON::Object::Ptr object_json=parser.extract<Poco::JSON::Object::Ptr>();
                        human.login=object_json->get("login").toString();
                        human.first_name=object_json->get("first_name").toString();
                        human.last_name=object_json->get("last_name").toString();
                        object_json->get("age").convert(human.age);
                    }
                    catch(...){
                        found=false;
                        cout<<"Parsing error: "<<message_load<<" bad json\n";
                        continue;
                    }
                    auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
                    auto &session=*createsession;
                    POCO_CHECKER(
                        Poco::Data::Statement database_request(session);
                        database_request<<"SELECT login FROM Person WHERE login=?",Poco::Data::Keywords::use(human.login),Poco::Data::Keywords::range(0, 1);
                        database_request.execute();
                        Poco::Data::RecordSet record_set(database_request);
                        if(!record_set.moveFirst()&&found){
                            Poco::Data::Statement database_request1(session);
                            database_request1<<"INSERT INTO Person (login, first_name, last_name, age) VALUES (?, ?, ?, ?)",Poco::Data::Keywords::use(human.login),Poco::Data::Keywords::use(human.first_name),Poco::Data::Keywords::use(human.last_name),Poco::Data::Keywords::use(human.age),Poco::Data::Keywords::range(0, 1);
                            database_request1.execute();
                        }
                    )
                    data_consumer.commit(message);
                }
            }
        }
    }
    catch(const exception &e){
        cerr<<e.what()<<"\n";
    }
    return 1;
}