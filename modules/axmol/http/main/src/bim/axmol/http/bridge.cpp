// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/http/bridge.hpp>

#include <iscool/http/request.hpp>
#include <iscool/http/setup.hpp>

#include <axmol/network/HttpClient.h>

static void send_request(iscool::http::request request)
{
  ax::network::HttpRequest* const ax_request(new ax::network::HttpRequest());

  ax_request->setUrl(request.url);

  switch (request.request_type)
    {
    case iscool::http::request::type::get:
      ax_request->setRequestType(ax::network::HttpRequest::Type::GET);
      break;
    case iscool::http::request::type::post:
      ax_request->setRequestType(ax::network::HttpRequest::Type::POST);
      ax_request->setHeaders(request.headers);
      ax_request->setRequestData(request.body.c_str(), request.body.size());
      break;
    }

  ax_request->setCompleteCallback(
      [=](ax::network::HttpClient*,
          ax::network::HttpResponse* response) -> void
        {
          const yasio::sbyte_buffer& ax_buffer = *response->getResponseData();
          std::span<const char> data(ax_buffer.data(), ax_buffer.size());

          request.result_handler(
              iscool::http::response(response->getResponseCode(), data));

          response->getHttpRequest()->release();
        });

  ax::network::HttpClient::getInstance()->send(ax_request);
}

bim::axmol::http::bridge::bridge()
{
  iscool::http::initialize(&::send_request);
}

bim::axmol::http::bridge::~bridge()
{
  iscool::http::finalize();
  ax::network::HttpClient::destroyInstance();
}
