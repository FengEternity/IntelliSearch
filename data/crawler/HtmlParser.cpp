#include "HtmlParser.h"
#include "WebEnginePage.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    HtmlParser::HtmlParser() : m_loadFinished(false)
    {
        // 构造函数实现
    }

    bool HtmlParser::needsDynamicCrawling(const QString &url, const QString &html) {
        // 首先使用静态分析方法
        // 如果静态分析结果非常明确（强特征匹配），则直接返回结果
        // 否则，在弱特征分析后，如果结果不确定，则使用JavaScript执行分析器进行进一步分析
        // 强特征：主流框架特征 - 扩展更多框架特征
        static const QStringList strongFeatures = {
            // 主流前端框架
            "__NEXT_DATA__",      // Next.js
            "ReactDOM.render(",    // React
            "react-dom",          // React
            "createRoot",         // React 18+
            "ng-app",             // Angular
            "angular.module",      // AngularJS
            "v-app",              // Vue.js
            "Vue.createApp",       // Vue 3
            "new Vue",            // Vue 2
            "svelte:options",      // Svelte
            "_svelte",            // Svelte编译标记
            "Alpine.data",        // Alpine.js
            "x-data",             // Alpine.js指令
            "Ember.Application",  // Ember.js
            "ember-view",         // Ember.js
            "Backbone.View",      // Backbone.js
            "Polymer.Element",    // Polymer
            "lit-element",        // LitElement
            
            // API和数据交互
            "fetch(\"/api/",       // API请求
            "XMLHttpRequest",      // AJAX
            "axios.",             // Axios HTTP客户端
            "socket.io",          // Socket.IO
            "websocket",          // WebSocket
            "graphql",            // GraphQL
            "apollo-client",      // Apollo GraphQL
            "relay",              // Relay
            
            // 状态管理
            "@angular/core",       // Angular Core
            "vuex",               // Vue状态管理
            "pinia",              // Vue 3状态管理
            "redux",              // React状态管理
            "createStore",        // Redux/Zustand
            "useReducer",         // React Hooks
            "useState",           // React Hooks
            "mobx",               // MobX
            "recoil",             // Recoil
            "jotai",              // Jotai
            "zustand",            // Zustand
            
            // 组件标记
            "data-reactroot",      // React Root
            "ng-controller",       // Angular Controller
            "data-reactid",        // React ID
            "__vue__",            // Vue实例标记
            "hydrate",            // React/Vue SSR
            
            // AI和聊天应用特征
            "chat-container",      // AI聊天界面
            "message-input",       // 消息输入框
            "chat-history",        // 聊天历史
            "chat-message",        // 聊天消息
            "ai-response",         // AI响应
            "chatbot",            // 聊天机器人
            "conversation-",       // 对话元素
            
            // 路由系统
            "router-outlet",      // Angular Router
            "router-view",        // Vue Router
            "BrowserRouter",      // React Router
            "useNavigate",        // React Router v6
            "useHistory",         // React Router v5
            "useLocation",        // React Router
            "useParams",          // React Router
            "useRouteMatch",      // React Router
            "history.pushState",  // History API
            
            // 动态UI库
            "ant-",               // Ant Design
            "mui-",               // Material-UI
            "chakra-",            // Chakra UI
            "tailwind-",          // Tailwind CSS
            "bootstrap-vue",      // Bootstrap Vue
            "ngb-",               // ng-bootstrap
            "el-",                // Element UI
            "v-calendar",         // Vue Calendar
            "react-datepicker",   // React Datepicker
            "swiper-",            // Swiper
            "slick-",             // Slick Carousel
            
            // 动态功能
            "lazy-load",          // 懒加载
            "infinite-scroll",    // 无限滚动
            "virtual-scroll",     // 虚拟滚动
            "drag-",              // 拖拽功能
            "drop-",              // 拖放功能
            "sortable",           // 排序功能
            "animation",          // 动画
            "transition",         // 过渡效果
            "toast-",             // 提示消息
            "modal-",             // 模态框
            "dialog-"             // 对话框
        };

        // 静态博客框架特征 - 扩展更多静态框架
        static const QStringList staticBlogFeatures = {
            "hexo-theme",         // Hexo
            "hugo-theme",         // Hugo
            "jekyll-theme",       // Jekyll
            "gatsby-theme",       // Gatsby
            "vuepress-theme",     // VuePress
            "docusaurus",         // Docusaurus
            "gridsome",           // Gridsome
            "nuxt-content",       // Nuxt Content
            "next-mdx",           // Next.js MDX
            "eleventy",           // 11ty
            "zola-",              // Zola
            "mkdocs-",            // MkDocs
            "docsify",            // Docsify
            "sphinx-",            // Sphinx
            "gitbook",            // GitBook
            "mdbook",             // mdBook
            "astro-",             // Astro
            "saber-theme",        // Saber
            "middleman-",         // Middleman
            "pelican-"            // Pelican
        };

        // 检查是否为静态博客
        for (const QString &feature : staticBlogFeatures) {
            if (html.contains(feature)) {
                DEBUGLOG("Static blog framework detected: {}", feature.toStdString());
                return false;
            }
        }

        // 检查强特征
        for (const QString &feature : strongFeatures) {
            if (html.contains(feature)) {
                DEBUGLOG("Strong framework feature detected: {}", feature.toStdString());
                return true;
            }
        }

        // DOM结构分析 - 扩展更多绑定模式
        int dynamicBindings = 0;
        // Vue, Angular, React事件绑定
        dynamicBindings += html.count(QRegularExpression("@click|@change|@input|v-on:|ng-click|onClick=|onChange=|onInput=|onSubmit=|onFocus=|onBlur="));
        // 数据绑定
        dynamicBindings += html.count(QRegularExpression("v-bind:|v-model|ng-model|\\[\\(.*?\\)\\]|\\[(.*?)\\]|\\{\\{.*?\\}\\}|\\{.*?\\}"));
        // React Hooks
        dynamicBindings += html.count(QRegularExpression("useState|useEffect|useContext|useReducer|useCallback|useMemo|useRef"));

        if (dynamicBindings > 20) { // 降低阈值以提高敏感度
            DEBUGLOG("High number of dynamic bindings detected: {}", dynamicBindings);
            return true;
        }

        // JavaScript代码分析 - 扩展更多API模式
        int ajaxPatterns = html.count(QRegularExpression("\\.ajax\\(|axios\\.|fetch\\(|new WebSocket\\(|new EventSource\\(|\\$\\.get\\(|\\$\\.post\\(|createObjectStore|indexedDB|localStorage"));
        if (ajaxPatterns > 0) {
            DEBUGLOG("AJAX/WebSocket/Storage API patterns detected: {}", ajaxPatterns);
            return true;
        }

        // 检测SPA路由机制
        int routerPatterns = html.count(QRegularExpression("history\\.push|history\\.replace|useNavigate|useHistory|useLocation|useParams|useRouteMatch|router\\.push|router\\.replace|\\$router\\.push|\\$router\\.replace"));
        if (routerPatterns > 0) {
            DEBUGLOG("SPA router patterns detected: {}", routerPatterns);
            return true;
        }

        // 检测状态管理库使用
        int stateManagementPatterns = html.count(QRegularExpression("createStore|useReducer|useContext|Provider|connect\\(|mapStateToProps|mapDispatchToProps|useSelector|useDispatch|combineReducers"));
        if (stateManagementPatterns > 0) {
            DEBUGLOG("State management patterns detected: {}", stateManagementPatterns);
            return true;
        }

        // 弱特征权重系统 - 优化权重分配
        double score = 0.0;

        // URL特征 (权重: 0.3) - 扩展
        if (url.contains(QRegularExpression("/(spa|api|graphql|chat|ai|app|dashboard|admin|portal|react|vue|angular)/", QRegularExpression::CaseInsensitiveOption))) {
            score += 0.3;
            DEBUGLOG("URL pattern suggests dynamic content: {}", url.toStdString());
        }

        // 模板语法 (权重: 0.3)
        if (html.contains(QRegularExpression("\\{\\{.*?\\}\\}|\\{\\[.*?\\]\\}|\\$\\{.*?\\}"))) {
            score += 0.3;
            DEBUGLOG("Template syntax detected");
        }

        // 脚本分析 (权重: 0.4)
        int scriptCount = html.count("<script");
        if (scriptCount > 5) { // 降低阈值
            score += 0.2;
            DEBUGLOG("High script count: {}", scriptCount);
            
            // 检查脚本内容中的动态特征
            if (html.contains(QRegularExpression("addEventListener|querySelector|getElementById|appendChild|innerHTML|textContent"))) {
                score += 0.2;
                DEBUGLOG("DOM manipulation detected in scripts");
            }
        }

        // 动态加载 (权重: 0.3)
        if (html.contains(QRegularExpression("require\\(\"|import\\s+.*?from|System\\.import|import\\(|dynamic import"))) {
            score += 0.3;
            DEBUGLOG("Dynamic module loading detected");
        }

        // 交互元素密度分析 (权重: 0.2)
        int interactiveElements = html.count(QRegularExpression("<(button|input|textarea|select|form|a href=\"javascript:|a onclick=)"));
        if (interactiveElements > 8) { // 降低阈值
            score += 0.2;
            DEBUGLOG("High interactive element count: {}", interactiveElements);
        }
        
        // 检测懒加载图片 (权重: 0.2)
        if (html.count(QRegularExpression("lazy(-| )load|data-src|loading=\"lazy\"|IntersectionObserver")) > 0) {
            score += 0.2;
            DEBUGLOG("Lazy loading detected");
        }
        
        // 检测无限滚动 (权重: 0.2)
        if (html.count(QRegularExpression("infinite(-| )scroll|load(-| )more|pagination")) > 0) {
            score += 0.2;
            DEBUGLOG("Infinite scroll or pagination detected");
        }
        
        // 检测WebSocket或实时更新 (权重: 0.3)
        if (html.count(QRegularExpression("WebSocket|EventSource|Server-Sent Events|socket\\.io|mqtt|stomp|real(-| )time")) > 0) {
            score += 0.3;
            DEBUGLOG("WebSocket or real-time updates detected");
        }

        // 总分超过0.6判定为动态页面 (降低阈值)
        if (score >= 0.6) {
            DEBUGLOG("Weak features score threshold met: {}", score);
            return true;
        }

        // 如果弱特征分析得分在中间区域（0.3-0.6），结果不确定，使用JavaScript执行分析器进一步分析
        if (score >= 0.3 && score < 0.6) {
            DEBUGLOG("Static analysis inconclusive (score: {}), using JS execution analyzer", score);
            return detectDynamicContentWithJSAnalyzer(url, 5000);
        }
        
        DEBUGLOG("Page classified as static, final score: {}", score);
        return false;
    }

    HtmlParser::~HtmlParser()
    {
        // 析构函数实现
    }

    CrawlResult HtmlParser::parseHtml(const QString &url, const QString &html)
    {
        CrawlResult result;
        result.url = url;
        result.timestamp = QDateTime::currentDateTime();

        // 提取标题
        QRegularExpression titleRegex("<title>([^<]*)</title>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch titleMatch = titleRegex.match(html);
        if (titleMatch.hasMatch())
        {
            result.title = titleMatch.captured(1).trimmed();
        }

        // 提取内容 (简单实现，移除HTML标签)
        QTextDocument doc;
        doc.setHtml(html);
        result.content = doc.toPlainText();

        // 提取链接
        result.links = extractLinks(url, html);

        INFOLOG("Parsed HTML for URL: {}", url.toStdString());
        INFOLOG("Parser result - Title: {}, Content length: {}, Links count: {}", 
            result.title.toStdString(), 
            result.content.length(), 
            result.links.size()
        );
        // 打印解析结果
        INFOLOG("Parser Content: {}", result.content.toStdString());


        return result;
    }

    QStringList HtmlParser::extractLinks(const QString &baseUrl, const QString &html)
    {
        QStringList links;
        QSet<QString> uniqueLinks;

        // 提取href属性
        QRegularExpression hrefRegex("href=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator i = hrefRegex.globalMatch(html);

        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            QString href = match.captured(1).trimmed();

            // 规范化URL
            QString normalizedUrl = normalizeUrl(baseUrl, href);

            // 检查URL是否有效且未被处理过
            if (!normalizedUrl.isEmpty() && !uniqueLinks.contains(normalizedUrl))
            {
                uniqueLinks.insert(normalizedUrl);
                links.append(normalizedUrl);
            }
        }

        return links;
    }

    QString HtmlParser::normalizeUrl(const QString &baseUrl, const QString &url, 
                                    bool followExternalLinks,
                                    const QStringList &allowedDomains,
                                    const QStringList &urlFilters)
    {
        // 创建QUrl对象
        QUrl qurl;
        if (url.startsWith("http://") || url.startsWith("https://"))
        {
            qurl = QUrl(url);
        }
        else
        {
            QUrl base(baseUrl);
            qurl = base.resolved(QUrl(url));
        }

        // 检查URL是否有效
        if (!qurl.isValid())
        {
            return QString();
        }

        // 移除片段(#后面的内容)
        qurl.setFragment(QString());

        // 获取规范化的URL字符串
        QString normalizedUrl = qurl.toString();

        // 检查是否应该跟随外部链接
        if (!followExternalLinks && !baseUrl.isEmpty())
        {
            QUrl baseQUrl(baseUrl);
            if (baseQUrl.host() != qurl.host())
            {
                return QString();
            }
        }

        // 检查是否在允许的域名列表中
        if (!allowedDomains.isEmpty())
        {
            bool allowed = false;
            for (const QString &domain : allowedDomains)
            {
                if (qurl.host().endsWith(domain, Qt::CaseInsensitive))
                {
                    allowed = true;
                    break;
                }
            }
            if (!allowed)
            {
                return QString();
            }
        }

        // 应用URL过滤规则
        if (!urlFilters.isEmpty())
        {
            for (const QString &filter : urlFilters)
            {
                QRegularExpression regex(filter);
                if (regex.match(normalizedUrl).hasMatch())
                {
                    return QString();
                }
            }
        }

        return normalizedUrl;
    }
    
    QWebEnginePage* HtmlParser::createWebPage()
    {
        // 创建自定义WebEnginePage实例
        WebEnginePage *page = new WebEnginePage();
        return page;
    }
    
    bool HtmlParser::waitForPageLoad(QWebEnginePage *page, int timeout)
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        
        // 连接页面加载完成信号
        QObject::connect(page, &QWebEnginePage::loadFinished, [&](bool success) {
            m_loadFinished = success;
            loop.quit();
        });
        
        // 连接超时信号
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            WARNLOG("Page load timeout");
            loop.quit();
        });
        
        // 启动超时计时器
        timer.start(timeout);
        
        // 等待页面加载完成或超时
        loop.exec();
        
        return m_loadFinished;
    }
    
    CrawlResult HtmlParser::parseDynamicHtml(const QString &url, int timeout)
    {
        INFOLOG("Parsing dynamic HTML for URL: {}", url.toStdString());
        
        CrawlResult result;
        result.url = url;
        result.timestamp = QDateTime::currentDateTime();
        
        // 创建WebEnginePage实例
        QWebEnginePage *page = createWebPage();
        
        // 加载URL
        page->load(QUrl(url));
        
        // 等待页面加载完成
        if (!waitForPageLoad(page, timeout))
        {
            ERRORLOG("Failed to load page: {}", url.toStdString());
            delete page;
            return result;
        }
        
        // 获取页面标题
        result.title = page->title();
        
        // 获取页面HTML内容
        QEventLoop loop;
        page->toHtml([&](const QString &html) {
            // 提取内容
            QTextDocument doc;
            doc.setHtml(html);
            result.content = doc.toPlainText();
            
            // 提取链接
            result.links = extractLinks(url, html);
            
            loop.quit();
        });
        
        // 等待HTML内容获取完成
        loop.exec();
        
        INFOLOG("Parsed dynamic HTML for URL: {}", url.toStdString());
        INFOLOG("Parser result - Title: {}, Content length: {}, Links count: {}", 
            result.title.toStdString(), 
            result.content.length(), 
            result.links.size()
        );
        
        // 清理资源
        delete page;
        
        return result;
    }

    bool HtmlParser::detectDynamicContentWithJSAnalyzer(const QString &url, int timeout)
    {
        INFOLOG("Detecting dynamic content with JS analyzer for URL: {}", url.toStdString());
        
        // 使用JavaScript执行分析器分析页面
        JSExecutionResult result = m_jsAnalyzer.analyzeExecution(url, timeout);
        
        // 根据分析结果判断是否为动态内容
        bool isDynamic = result.hasDynamicContent;
        
        // 记录详细分析结果
        DEBUGLOG("JS Analyzer results - DOM mutations: {}, AJAX requests: {}, Event listeners: {}, Timers: {}, Storage access: {}",
                result.domMutationCount,
                result.ajaxRequestCount,
                result.eventListenerCount,
                result.timerCount,
                result.storageAccessCount);
        
        INFOLOG("JS Analyzer dynamic content detection result: {}", isDynamic ? "true" : "false");
        
        return isDynamic;
    }

} // namespace IntelliSearch