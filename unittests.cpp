#include <gtest/gtest.h>
#include "GlobData.hpp"

TEST(create, u_test){
    testing::internal::CaptureStdout();
    auto createsession=Create_Session();
    Poco::Data::Session &ses=*createsession;
    ifstream database("init_db.sql");
    char first[40],second[300];
    database.getline(first,40,'\n');
    database.getline(second,300,'\n');
    POCO_CHECKER(
        Poco::Data::Statement sfirst(ses),ssecond(ses);
        sfirst<<string(first); 
        ssecond<<string(second);
        sfirst.execute();
        ssecond.execute();
    )
    database.close();
    delete createsession;
    ASSERT_TRUE(testing::internal::GetCapturedStdout() == "");
}

TEST(test, u_test){
    vector<Person> test_data={{"Pasalc", "Alexandr", "Titeev", 22},{"polker", "Jim", "John", 12},{"amongus", "Dmitryi", "Child", 6},{"Pluska","Zinaida","Petrova",53},{"Printer","Vzzzzz","KHKHKHKHKH",6},{"Zxhdh","lhjrwehurt","djhjkhrwuh",36565},{"gIt","Gevorgyi","Popov",78},{"klown","HONK","HONK",1337},{"frog","kwa","frog",2},{"SAH","Sergey","Andreev",16},{"Mimka","Viacheslav","Grinin",23},{"dim","Dmitriy","Dybov",49},{"kow","Ann","Kovsh",25},{"fjhsdfjfjfffgu47v3iovBbh336yr9ob","BZHgGRlnrjlt745V^&&^vfrout*jff&Y",",mvpi^4587n7$9tb!3489ny7tny43t7v",9999}};
    for(int i=0;i<test_data.size();i++){
        Poco::Net::SocketAddress socket_address(IP, PORT);
        Poco::Net::StreamSocket stream_socket(socket_address);
        Poco::Net::SocketStream socket_stream(stream_socket);
        socket_stream<<"POST /person HTTP/1.1\ncontent-type: application/url-encoded\n\nadd=True&login="<<test_data[i].login<<"&first_name="<<test_data[i].first_name<<"&last_name="<<test_data[i].last_name<<"&age="<<test_data[i].age<<"\n";
        socket_stream.flush();
    }
    sleep(8);
    testing::internal::CaptureStdout(); 
    auto createsession=Create_Session();
    Poco::Data::Session &ses=*createsession;
    for(int i=0;i<test_data.size();i++){
        Person check_in_person;
        POCO_CHECKER(
            Poco::Data::Statement check_in(ses);
            check_in<<"SELECT login, first_name, last_name, age FROM Person WHERE login=? AND first_name=? AND last_name=? AND age=?;", Poco::Data::Keywords::into(check_in_person.login), Poco::Data::Keywords::into(check_in_person.first_name), Poco::Data::Keywords::into(check_in_person.last_name), Poco::Data::Keywords::into(check_in_person.age), Poco::Data::Keywords::use(test_data[i].login), Poco::Data::Keywords::use(test_data[i].first_name), Poco::Data::Keywords::use(test_data[i].last_name), Poco::Data::Keywords::use(test_data[i].age), Poco::Data::Keywords::range(0, 1);
            check_in.execute();
            Poco::Data::RecordSet record_set(check_in);
            ASSERT_TRUE(record_set.moveFirst());
        )
    }
    delete createsession;
    ASSERT_TRUE(testing::internal::GetCapturedStdout() == "");
}

int main(int argc, char *argv[]){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}