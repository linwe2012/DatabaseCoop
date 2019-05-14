
var user_el = $id('ipt-username')
var pass_el = $id('ipt-passwd')
var baseurl = 'http://127.0.0.1:3000/account/'
var show_password = false;

$id('password-lock').onclick = function(){
    show_password = !show_password;
    if(show_password){
        $id('password-lock').innerHTML = 'lock_open'
        pass_el.type = 'text'
    }
    else {
        $id('password-lock').innerHTML = 'lock'
        pass_el.type = 'password'
    }
}

function error_handle(what) {
    var user_w = document.getElementById('ipt-username-wrap')
    var pass_w = document.getElementById('ipt-passwd-wrap')
    user_w.style.border = 'none';
    pass_w.style.border = 'none';
    if(what.part == 'username') {
        user_w.style.border = '1px solid #e25c02';
    }
    else if(what.part == 'password') {
        pass_w.style.border = '1px solid #e25c02';
    }
    if(what.msg) {
        var msg = document.getElementById('error-msg')
        msg.style.color = ''
        msg.innerHTML =what.msg
    }
}

document.getElementById('user-login').onclick = async function() {
    var response = await $post({
        url: baseurl + 'signin',
        params: `username=${user_el.value}&password=${pass_el.value}`,
    });

    var res = JSON.parse(response)
    console.log(res)
    if(res.ok != true) {
        error_handle(res);
        return
    }
    window.location = '/user'
    
}

$id('user-signup').onclick = async function() {
    var response = await $post({
        url: baseurl + 'signup',
        params: `username=${user_el.value}&password=${pass_el.value}`,
    });

    var res = JSON.parse(response)
    console.log(res)
    if(res.ok != true) {
        error_handle(res);
        return
    }
    var msg = document.getElementById('error-msg')
    msg.innerHTML='Signup success! Please Login.'
    msg.style.color = '#008f94'
}