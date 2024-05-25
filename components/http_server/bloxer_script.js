const xhr = new XMLHttpRequest();
xhr.open('GET', 'bloxer_config.json', true);
xhr.responseType = 'json';

let config;
var verified;

// 获取用户名和密码输入框的引用
const usernameInput = document.getElementById('username');
const passwordInput = document.getElementById('password');

const loginForm = document.getElementById('login-form');

xhr.onload = function() {
    if (xhr.status === 200) 
    {
        // 获取配置文件中的用户名和密码
        config = xhr.response;
    } 
    else 
    {
        // fail
        console.error('Failed to load config file');
    }
};

loginForm.addEventListener('submit', function(event) 
{
    event.preventDefault();

    // 获取输入的用户名和密码
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;

    // 检查用户名和密码是否正确
    if (username === config.web_username && password === config.web_password) 
    {
        // 如果正确，跳转到 Dashboard/main 页面
        verified = true;
        window.location.href = 'bloxer_dashboard_setting.html';
    } 
    else 
    {
        // 如果不正确，弹出错误信息提示框
        verified = false;
        alert('用户名或密码错误！');
    }
});

// 发送请求
xhr.send();
