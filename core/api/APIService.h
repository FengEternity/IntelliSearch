#ifndef INTELLISEARCH_APISERVICE_H
#define INTELLISEARCH_APISERVICE_H

#include <string>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

class APIService {
public:
    virtual ~APIService() = default;

    // 获取服务名称
    virtual std::string getServiceName() const = 0;

    // 检查服务是否可用
    virtual bool isAvailable() const = 0;

    // 获取服务优先级（用于负载均衡和故障转移）
    virtual int getPriority() const = 0;

protected:
    // API调用的通用错误处理
    virtual void handleError(const std::string& error) = 0;

    // 验证API密钥
    virtual bool validateApiKey() const = 0;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_APISERVICE_H