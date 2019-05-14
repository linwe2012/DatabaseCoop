const bcrypt = require('bcrypt')
const Validator = require('../check-valid-input')

function validate(name, password, ctx) {
    if(!Validator.username(name)){
        ctx.response.body = {ok: false, part: 'username', msg: 'Name must be larger than 3'}
        return false
    }
    if(!Validator.password(password)){
        ctx.response.body = {ok: false, part: 'password', msg: 'Passwd must be larger than 3'}
        return false
    }
    return true
}


function signin(ctx, next){
    var
    name = ctx.request.body.username || '',
    password = ctx.request.body.password || '';
    ctx.response.type = 'json'

    if(!validate(name, password, ctx)) return;

    conn.query(
        sqlhandles.login,
        [name],
        function(err, rows, fields) {
            if(err.retcode != 0 || rows.length == 0) {
                ctx.response.body = {ok: false, part: 'username', msg: 'User name or password mismatch.'}
                return
            }

            var isMatch = bcrypt.compareSync(password, rows[0].usr_passwd)
            if(!isMatch) {
                ctx.response.body = {ok: false, part: 'password', msg: 'User name or password mismatch.'}
            }
            else {
                ctx.response.body = {ok: true}
                ctx.session.usr_name = rows[0].usr_name
                ctx.session.max_num_book_allowed = rows[0].max_num_book_allowed
                ctx.session.max_renew_allowed = rows[0].max_renew_allowed
                ctx.session.usr_avatar = rows[0].usr_avatar
                // ctx.response.redirect('/user')
            }
        }
    )
}

function signup(ctx, next) {

    var internal_error = {
        ok: false, part: 'username', msg: 'Internal Error: '
    }

    var
    name = ctx.request.body.username || '',
    password = ctx.request.body.password || '';
    ctx.response.type = 'json'
    var failed = false
    conn.query( sqlhandles.login,
        [name], function(err, rows, fields){
        if(rows.length != 0) {
            ctx.response.body = {ok: false, part: 'username', msg: 'User name already taken.'}
            failed = true
        }
    })
    if(failed) return

    var pass = bcrypt.hashSync(password, 10 /* salt round */);

    conn.query( sqlhandles.new_user, [name, pass], function(err, rows) {
        if(err.retcode != 0) { 
            internal_error.msg += 'Unable to insert into database'; ctx.response.body = internal_error 
            console.log('Insert new user failed:')
            console.log(err)
        }
        else { 
            ctx.response.body = { ok: true}
        }
    })
}

module.exports = {
    'GET /account' : (ctx, next)=>{
        ctx.render('account.html', {
        })
    },

    'POST /account/signin' : signin,

    'POST /account/signup' : signup

}