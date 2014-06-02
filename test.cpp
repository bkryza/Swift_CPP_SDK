#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <json/json.h>
#include <iostream>
#include "io/HTTPIO.h"
#include "model/Account.h"


using namespace Poco::Net;
using namespace Poco;
using namespace std;
using namespace Swift;

int main(int argc, char** argv)
{
  /*Poco::Net::SocketAddress sa("www.appinf.com", 80);
  Poco::Net::StreamSocket socket(sa);
  Poco::Net::SocketStream str(socket);*/

  /*HTTPClientSession session("www.google.com");
  HTTPRequest request(HTTPRequest::HTTP_GET);
  session.sendRequest(request);

  HTTPResponse response;
  std::istream& rs = session.receiveResponse(response);
  StreamCopier::copyStream(rs,std::cout);


  Json::Value root;
  Json::Reader reader;
  const char *temp = "{\"firstName\": \"John\",\"lastName\": \"Smith\"}";
  reader.parse(temp,root,false);
  std::string encoding = root.get("firstName", "Not Found" ).asString();
  std::cout<<std::endl<<encoding<<std::endl;

  Json::Value mydoc;
  Json::Value mydoc2;
  mydoc2["key1"] = "sdfsdf";
  //key1 {"sdfds",{"ssf"}}

  mydoc["key1"] = {};
  mydoc["key1"]["name"] = "jafar";
  mydoc["key2"] = "value 2";
  mydoc["key3"]["key3_1"] = "value 1_1";
  mydoc["key3"]["key31"] = "value 1_1";
  mydoc["key1"]["sdf"] = "sdfsdf";
  Json::StyledWriter writer;
  // Make a new JSON document for the configuration. Preserve original comments.
  std::string outputConfig = writer.write( mydoc );

  // And you can write to a stream, using the StyledWriter automatically.
  std::cout << mydoc;
  */

  /*
    {
      "auth":
        {
          "tenantName": "BehroozProject",
          "passwordCredentials":
            {
              "username": "behrooz",
              "password": "behrooz"
            }
        }
    }
   */
  Account* account = new Account();
  account->authenticate("behrooz","behrooz","http://192.168.249.109:5000/v2.0/tokens",true,"BehroozProject");
  //account->increaseCallCounter();
}
