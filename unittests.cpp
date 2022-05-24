#include <gtest/gtest.h>
#include "GlobData.hpp"

void Test_Add(Person test_data){
    Poco::Net::SocketAddress socket_address(IP, PORT);
    Poco::Net::StreamSocket stream_socket(socket_address);
    Poco::Net::SocketStream socket_stream(stream_socket);
    socket_stream<<"POST /person HTTP/1.1\ncontent-type: application/url-encoded\n\nlogin="<<test_data.login<<"&first_name="<<test_data.first_name<<"&last_name="<<test_data.last_name<<"&age="<<test_data.age<<"\n";
    socket_stream.flush();
    stream_socket.shutdownSend();
}

bool Test_Find(Person test_data){
    Poco::Net::SocketAddress socket_address(IP, PORT);
    Poco::Net::StreamSocket stream_socket(socket_address);
    Poco::Net::SocketStream socket_stream(stream_socket);
    socket_stream<<"GET /person?login="<<test_data.login<<"\nHTTP/1.1\n"; 
    socket_stream.flush();
    stream_socket.shutdownSend();
    stringstream copy_stream;
    Poco::StreamCopier::copyStream(socket_stream, copy_stream);
    string for_search=copy_stream.str();
    int first=for_search.find('{'),last=for_search.find('}');
    string json=for_search.substr(first,last-first+1);
    Poco::JSON::Parser tmp;
    Poco::Dynamic::Var poco_var=tmp.parse(json);
    Poco::JSON::Object::Ptr object_json=poco_var.extract<Poco::JSON::Object::Ptr>();
    string login=object_json->get("login").toString();
    string first_name=object_json->get("first_name").toString();
    string last_name=object_json->get("last_name").toString();
    int age;
    object_json->get("age").convert(age);
    return (login==test_data.login)&&(first_name==test_data.first_name)&&(last_name==test_data.last_name)&&(age==test_data.age);  
}

TEST(create, u_test){
    testing::internal::CaptureStdout();
    auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
    auto &ses=*createsession;
    ifstream database("init_db.sql");
    char first[40],second[300];
    database.getline(first,40,'\n');
    database.getline(second,300,'\n');
    database.close();
    POCO_CHECKER(
        Poco::Data::Statement sfirst(ses),ssecond(ses);
        sfirst<<string(first); 
        ssecond<<string(second);
        sfirst.execute();
        ssecond.execute();
    )
    ASSERT_TRUE(testing::internal::GetCapturedStdout()=="");
}

TEST(test_add, u_test){
    vector<Person> test_data={{"Pasalc", "Alexandr", "Titeev", 22},{"polker", "Jim", "John", 12},{"amongus", "Dmitryi", "Child", 6},{"Pluska","Zinaida","Petrova",53},{"Printer","Vzzzzz","KHKHKHKHKH",6},{"Zxhdh","lhjrwehurt","djhjkhrwuh",36565},{"gIt","Gevorgyi","Popov",78},{"klown","HONK","HONK",1337},{"frog","kwa","frog",2},{"SAH","Sergey","Andreev",16},{"Mimka","Viacheslav","Grinin",23},{"dim","Dmitriy","Dybov",49},{"kow","Ann","Kovsh",25},{"fjhsdfjfjfffgu47v3iovBbh336yr9ob","BZHgGRlnrjlt745V^&&^vfrout*jff&Y",",mvpi^4587n7$9tb!3489ny7tny43t7v",9999}};
    testing::internal::CaptureStdout();
    for(int i=0;i<test_data.size();i++){
        Test_Add(test_data[i]);
    }
    sleep(8);
    for(int i=0;i<test_data.size();i++){
        if(!Test_Find(test_data[i])){
            cout<<"No user with:\nlogin="<<test_data[i].login<<"\nfirst_name="<<test_data[i].first_name<<"\nlast_name="<<test_data[i].last_name<<"\nage="<<test_data[i].age<<"\n";
        }
    }
    ASSERT_TRUE(testing::internal::GetCapturedStdout() == "");
}

int main(int argc, char *argv[]){
    cout<<"Enter ip: ";
    cin>>IP;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
