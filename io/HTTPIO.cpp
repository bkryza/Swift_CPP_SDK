/*
 * HTTPIO.cpp
 *
 *  Created on: 2014-05-27
 *      Author: Behrooz Shafiee Sarjaz
 */

#include "HTTPIO.h"
#include <sstream>      // ostringstream

namespace Swift {

using namespace std;
using namespace Poco::Net;
using namespace Poco;

Poco::Net::HTTPClientSession* doHTTPIO(const Poco::URI& uri,
    const std::string& type, std::vector<HTTPHeader>* params) {
  Poco::Net::HTTPClientSession *session = new HTTPClientSession(uri.getHost(),
      uri.getPort());
  Poco::Net::HTTPRequest request(type, uri.getPathAndQuery());

  //Add params
  if (params != nullptr && params->size() > 0) {
    vector<HTTPHeader>::iterator it = params->begin();
    while (it != params->end()) {
      request.add(it->getKey(), it->getValue());
      it++;
    }
  }
  //request.write(cout);
  session->sendRequest(request);
  return session;
}

Poco::Net::HTTPClientSession* doHTTPIO(const Poco::URI& uri,
    const std::string& type, std::vector<HTTPHeader>* params,
    const std::string& reqBody, const std::string& contentType) {
  HTTPClientSession *session = new HTTPClientSession(uri.getHost(),
      uri.getPort());
  HTTPRequest request(type, uri.getPath());
  //Set Content Type
  request.setContentLength(reqBody.size());
  request.setContentType(contentType);

  //Add params
  if (params != nullptr && params->size() > 0) {
    vector<HTTPHeader>::iterator it = params->begin();
    while (it != params->end()) {
      request.add(it->getKey(), it->getValue());
      it++;
    }
  }

  //write request body
  ostream &ostream = session->sendRequest(request);
  if (ostream == nullptr)
    return nullptr;
  ostream << reqBody;
  return session;
}

Poco::Net::HTTPClientSession* doHTTPIO(const Poco::URI& uri,
    const std::string& type, std::vector<HTTPHeader>* params,
    const char* reqBody, ulong size) {
  HTTPClientSession *session = new HTTPClientSession(uri.getHost(),
      uri.getPort());
  HTTPRequest request(type, uri.getPath());
  //Set Content size
  request.setContentLength(size);

  //Add params
  if (params != nullptr && params->size() > 0) {
    vector<HTTPHeader>::iterator it = params->begin();
    while (it != params->end()) {
      request.add(it->getKey(), it->getValue());
      it++;
    }
  }

  //write request body
  ostream &ostream = session->sendRequest(request);
  if (ostream == nullptr)
    return nullptr;
  ostream << reqBody;
  return session;
}

Poco::Net::HTTPClientSession* doHTTPIO(const Poco::URI& uri,
    const std::string& type, std::vector<HTTPHeader>* params,
    std::istream& inputStream) {
  HTTPClientSession *session = new HTTPClientSession(uri.getHost(),
        uri.getPort());
  HTTPRequest request(type, uri.getPath());

  //Add params
  if (params != nullptr && params->size() > 0) {
    vector<HTTPHeader>::iterator it = params->begin();
    while (it != params->end()) {
      request.add(it->getKey(), it->getValue());
      it++;
    }
  }

  //write request body
  ostream &ostream = session->sendRequest(request);
  if (ostream == nullptr)
    return nullptr;
  //Copy input stream to the output stream
  StreamCopier::copyStream(inputStream,ostream);
  //ostream << inputStream;

  return session;
}


template<class T>
inline SwiftResult<T>* returnNullError(const string &whatsNull) {
  SwiftResult<T> *result = new SwiftResult<T>();
  SwiftError error(SwiftError::SWIFT_FAIL, whatsNull + " is NULL");
  result->setError(error);
  result->setResponse(nullptr);
  result->setPayload(nullptr);
  return result;
}

/** Template instantiation for common used types **/
//template
//SwiftResult<istream*>* doSwiftTransaction(Account *_account,URI &_uri,string &_method,std::vector<HTTPHeader>* _uriParams,std::vector<HTTPHeader>* _reqMap, vector<int> _validHTTPCodes);

template
SwiftResult<void*>* doSwiftTransaction<void*>(Account *_account,URI *_uri,string *_method,std::vector<HTTPHeader>* _uriParams,std::vector<HTTPHeader>* _reqMap);//{}


template<class T>
SwiftResult<T>* doSwiftTransaction(Account *_account,URI *_uri,string *_method,std::vector<HTTPHeader>* _uriParams,std::vector<HTTPHeader>* _reqMap) {
  if (_account == nullptr)
    return returnNullError<T>("account");
  Endpoint* swiftEndpoint = _account->getSwiftService()->getFirstEndpoint();
  if (swiftEndpoint == nullptr)
    return returnNullError<T>("SWIFT Endpoint");

  //Create parameter map
  vector<HTTPHeader> *reqParamMap = new vector<HTTPHeader>();
  //Add authentication token
  string tokenID = _account->getToken()->getId();
  reqParamMap->push_back(*new HTTPHeader("X-Auth-Token", tokenID));
  //Add rest of request Parameters
  if (_reqMap!=nullptr&& _reqMap->size() > 0) {
    for (uint i = 0; i < _reqMap->size(); i++) {
      reqParamMap->push_back(_reqMap->at(i));
    }
  }

  if(_uriParams!=nullptr && _uriParams->size()>0) {
    //Create appropriate URI
    ostringstream queryStream;
    queryStream << "?";
    if (_uriParams != nullptr && _uriParams->size() > 0) {
      for (uint i = 0; i < _uriParams->size(); i++) {
        if (i > 0)
          queryStream << ",";
        queryStream << _uriParams->at(i).getQueryValue();
      }
    }
    _uri->setQuery(queryStream.str());
  }

  //Creating HTTP Session
  HTTPResponse *httpResponse = new HTTPResponse();
  istream* resultStream = nullptr;
  try {
    /** This operation does not accept a request body. **/
    HTTPClientSession *httpSession = doHTTPIO(*_uri, HTTPRequest::HTTP_POST,reqParamMap);
    resultStream = &httpSession->receiveResponse(*httpResponse);
  } catch (Exception &e) {
    SwiftResult<T> *result = new SwiftResult<T>();
    SwiftError error(SwiftError::SWIFT_EXCEPTION, e.displayText());
    result->setError(error);
    //Try to set HTTP Response as the payload
    result->setResponse(httpResponse);
    result->setPayload(nullptr);
    return result;
  }

  /**
   * Check HTTP return code
   */
  bool valid = false;
  //for(uint i=0;i<_)
  if (httpResponse->getStatus() != HTTPResponse::HTTP_NO_CONTENT) {
    SwiftResult<T> *result = new SwiftResult<T>();
    SwiftError error(SwiftError::SWIFT_HTTP_ERROR, httpResponse->getReason());
    result->setError(error);
    result->setResponse(httpResponse);
    result->setPayload(nullptr);
    return result;
  }
  //Everything seems fine
  SwiftResult<T> *result = new SwiftResult<T>();
  result->setError(SWIFT_OK);
  result->setResponse(httpResponse);
  result->setPayload(resultStream);
  return result;
}


} /* namespace Swift */
