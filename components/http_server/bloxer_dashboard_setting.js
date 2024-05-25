const xhr = new XMLHttpRequest();
xhr.open('GET', 'bloxer_config.json', true);
xhr.responseType = 'json';

let config;

const settingForm = document.getElementById('setting-form');

xhr.onload = function() {
    if (xhr.status === 200) 
    {
        config = xhr.response;
        updateInput();
    } 
    else 
    {
        // fail
        console.error('Failed to load config file');
    }
};
// 发送请求
xhr.send();

function updateInput() {
    // 获取输入框元素
    var input_wifi_ssid = document.getElementById('wifi_ssid');
    var input_wifi_password = document.getElementById('wifi_password');
    var input_ddns_domain = document.getElementById('ddns_domain');
    var input_ddns_token = document.getElementById('ddns_token');
    var input_web_username = document.getElementById('web_username');
    var input_web_password = document.getElementById('web_password');
    var input_repo_type = document.getElementById('repo_type');
    var input_repo_owner = document.getElementById('repo_owner');
    var input_repo_name = document.getElementById('repo_name');
    // 修改输入框的值
    input_wifi_ssid.value = config.wifi_ssid;
    input_wifi_password.value = config.wifi_password;
    input_ddns_domain.value = config.ddns_domain;
    input_ddns_token.value = config.ddns_token;
    input_web_username.value = config.web_username;
    input_web_password.value = config.web_password;
    input_repo_type.value = config.repo_type;
    input_repo_owner.value = config.repo_owner;
    input_repo_name.value = config.repo_name;
}

document.getElementById('saveButton').addEventListener('click', function(event)
{
    event.preventDefault();

    // 收集输入框的值
    var formData = new FormData(settingForm);
    var urlParams = new URLSearchParams();
    
    for (var pair of formData.entries()) {
        if (pair[1].trim() !== "") { // 如果输入值不为空
            urlParams.append(pair[0], pair[1]); // 将输入值添加到 URL 参数中
        }
    }
    
    // 构造请求 URL
    var url = '/bloxer_modify?' + urlParams.toString();

    console.log(url);
    
    var xhttp = new XMLHttpRequest();
    // 设置请求完成时的回调函数
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == XMLHttpRequest.DONE) {
            if (xhttp.status == 200) {
                // 请求成功，处理响应数据
                console.log(xhttp.responseText);
            } else {
                // 请求失败，输出错误信息
                console.error('Request failed:', xhttp.status);
            }
        }
    };

    // 打开并发送 GET 请求
    xhttp.open('GET', url, true);
    xhttp.send();
});


document.getElementById('uploadButton').addEventListener('click', function(event)
{
    event.preventDefault();

    var xhttp = new XMLHttpRequest();
    // 设置请求完成时的回调函数
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == XMLHttpRequest.DONE) {
            if (xhttp.status == 200) {
                // 请求成功，处理响应数据
                console.log(xhttp.responseText);
            } else {
                // 请求失败，输出错误信息
                console.error('Request failed:', xhttp.status);
            }
        }
    };

    xhttp.open("POST", "/upload/", true);
    xhttp.send();
});