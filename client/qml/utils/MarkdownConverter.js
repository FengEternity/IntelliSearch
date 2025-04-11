function convertMarkdownToHtml(markdown) {
    if (!markdown) return "";
    
    // 处理加粗文本
    let html = markdown.replace(/\*\*(.*?)\*\*/g, '<b>$1</b>');
    
    // 处理换行符
    html = html.replace(/\\n/g, '<br>');
    
    // 处理有序列表
    html = html.replace(/(\d+)\.\s+(.*?)(?=(?:\d+\.|$))/g, '<p>$1. $2</p>');
    
    // 处理链接
    html = html.replace(/\[(.*?)\]\((.*?)\)/g, '<a href="$2" style="color: #007AFF;">$1</a>');
    
    // 处理代码块
    html = html.replace(/`(.*?)`/g, '<code style="background-color: #f0f0f0; padding: 2px 4px; border-radius: 3px;">$1</code>');
    
    return html;
}