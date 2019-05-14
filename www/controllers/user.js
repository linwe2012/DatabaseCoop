module.exports = {
    'GET /user': async (ctx, next)=>{
        if(!ctx.session.usr_name) {
            ctx.response.redirect('/account')
            return
        }
        
        ctx.render('user.html', {
            avatar : ctx.session.usr_avatar || '/static/img/avatar.png',
            usr_name: ctx.session.usr_name
        })
    }
}