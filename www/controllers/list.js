module.exports = {
    'GET /list': async (ctx, next)=>{
        // conn.query('select * from book')
        ctx.render('list.html', {
            avatar : '/static/img/avatar.png'
        })
    },

    'GET /branch': async (ctx, next)=>{
        ctx.render('branch.html', {
            avatar : '/static/img/avatar.png'
        })
    }
}