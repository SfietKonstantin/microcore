#include "httprequestfactory.h"
#include <QNetworkReply>

using namespace ::microcore::core;

namespace microcore { namespace http {

class HttpGetRequestJob: public HttpJob
{
public:
    explicit HttpGetRequestJob(QNetworkAccessManager &network, HttpRequest &&request)
        : m_network(network)
        , m_request(std::move(request))
    {
    }
    void execute(ICallback &callback) override
    {
        QNetworkReply *reply = nullptr;
        switch (m_request.type()) {
        case HttpRequest::Type::Get:
            reply = m_network.get(m_request.request());
            break;
        case HttpRequest::Type::Post:
            reply = m_network.post(m_request.request(), m_request.postData());
            break;
        case HttpRequest::Type::Delete:
            reply = m_network.deleteResource(m_request.request());
            break;
        default:
            break;
        }
        Q_ASSERT_X(reply != nullptr, "execute", "HttpRequest::Type must be Get, Post or Delete");

        reply->setParent(nullptr);
        m_result.reset(reply);

        QObject::connect(reply, &QNetworkReply::finished, [this, reply, &callback]() {
            if (reply->error() != QNetworkReply::NoError) {
                callback.onError(reply->errorString());
            } else {
                callback.onResult(std::move(m_result));
            }
        });
    }
private:
    QNetworkAccessManager &m_network;
    HttpRequest m_request {};
    HttpResult m_result {};
};

HttpRequestFactory::HttpRequestFactory(QNetworkAccessManager &network)
    : m_network(network)
{
}

std::unique_ptr<HttpJob> HttpRequestFactory::create(HttpRequest &&request) const
{
    return std::unique_ptr<HttpJob>(new HttpGetRequestJob(m_network, std::move(request)));
}

}}
