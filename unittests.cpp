#include <gtest/gtest.h>
#include "GlobData.hpp"

void Test_Create(string first, string second){
    auto createsession=unique_ptr<Poco::Data::Session>(Create_Session());
    auto &ses=*createsession;
    POCO_CHECKER(
        Poco::Data::Statement sfirst(ses),ssecond(ses);
        sfirst<<first; 
        ssecond<<second;
        sfirst.execute();
        ssecond.execute();
    )
}

void Test_Add(Person test_data){
    Poco::Net::SocketAddress socket_address(IP, PORT);
    Poco::Net::StreamSocket stream_socket(socket_address);
    Poco::Net::SocketStream socket_stream(stream_socket);
    socket_stream<<"POST /person HTTP/1.1\ncontent-type: application/url-encoded\n\nlogin="<<test_data.login<<"&first_name="<<test_data.first_name<<"&last_name="<<test_data.last_name<<"&age="<<test_data.age<<"\n";
    socket_stream.flush();
    stream_socket.shutdownSend();
}

void Test_Find(Person test_data, int *data){
    *data=1;
    Poco::Net::SocketAddress socket_address(IP, PORT);
    Poco::Net::StreamSocket stream_socket(socket_address);
    Poco::Net::SocketStream socket_stream(stream_socket);
    socket_stream<<"GET /person?login="<<test_data.login<<"\nHTTP/1.1\n";
    socket_stream.flush();
    stream_socket.shutdownSend();
    stringstream copy_stream;
    Poco::StreamCopier::copyStream(socket_stream, copy_stream);
    vector<char> char_stream(512);
    string for_search;
    while(copy_stream){
        copy_stream.getline(char_stream.data(), char_stream.size(), '\n');
        for_search += char_stream.data();
    }
    string age = to_string(test_data.age);
    if(search(for_search.begin(), for_search.end(), test_data.login.begin(), test_data.login.end())==for_search.end()||search(for_search.begin(), for_search.end(), test_data.first_name.begin(), test_data.first_name.end())==for_search.end()||search(for_search.begin(), for_search.end(), test_data.last_name.begin(), test_data.last_name.end())==for_search.end()||search(for_search.begin(), for_search.end(), age.begin(), age.end())==for_search.end()){
        *data = 0;
    }
}

TEST(create, u_test){
    ifstream database("init_db.sql");
    char first[40],second[300];
    database.getline(first,40,';');
    database.getline(second,300,';');
    string first_s(first),second_s(second),end_line=" -- sharding:";
    first_s+=end_line;
    second_s+=end_line;
    database.close();
    testing::internal::CaptureStdout();
    vector<thread*> threads(DATABASE_SHARDS);
    for(int i=0;i<threads.size();i++){
        threads[i] = new thread(Test_Create, first_s+to_string(i), second_s+to_string(i));
    }
    for(int i=0;i<threads.size();i++){
        threads[i]->join(); 
        delete threads[i];
    }
    ASSERT_TRUE(testing::internal::GetCapturedStdout()=="");
}

TEST(test_add, u_test){
    vector<Person> test_data={{"Pasalc", "Alexandr", "Titeev", 22},{"polker", "Jim", "John", 12},{"amongus", "Dmitryi", "Child", 6},{"Pluska","Zinaida","Petrova",53},{"Printer","Vzzzzz","KHKHKHKHKH",6},{"Zxhdh","lhjrwehurt","djhjkhrwuh",36565},{"gIt","Gevorgyi","Popov",78},{"klown","HONK","HONK",1337},{"frog","kwa","frog",2},{"SAH","Sergey","Andreev",16},{"Mimka","Viacheslav","Grinin",23},{"dim","Dmitriy","Dybov",49},{"kow","Ann","Kovsh",25},{"fjhsdfjfjfffgu47v3iovBbh336yr9ob","BZHgGRlnrjlt745V^&&^vfrout*jff&Y",",mvpi^4587n7$9tb!3489ny7tny43t7v",9999}};
    vector<thread*> threads(test_data.size());
    testing::internal::CaptureStdout();
    for(int i=0;i<threads.size();i++){
        threads[i]=new thread(Test_Add, test_data[i]);
    }
    for(int i=0;i<threads.size();i++){
        threads[i]->join(); 
        delete threads[i];
    }
    sleep(8); 
    vector<int> test(test_data.size()); 
    for(int i=0;i<threads.size();i++){
        threads[i]=new thread(Test_Find, test_data[i], test.data()+i);
    }
    for(int i=0;i<threads.size();i++){
        threads[i]->join(); 
        delete threads[i];
    }
    ASSERT_TRUE(testing::internal::GetCapturedStdout() == "");
    for(int i=0;i<test.size();i++){
        ASSERT_TRUE(test[i]);
    }
}

int main(int argc, char *argv[]){
    cout<<"Enter ip: ";
    cin>>IP;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
