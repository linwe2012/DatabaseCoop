const UserPriv = require('./internal/userprivvalidate')

function execsql(ctx, next) {
    if(!UserPriv.isAdmin(ctx, next)) {
        ctx.response.status = httpstatus.forbidden
        return
    }
    let u = ctx.request.body.sql
    if(u === null || u == undefined) {
        ctx.response.body = {
            ok: false,
            msg: 'invalid sql stmt'
        }
        return
    }
    conn.query(u, (err, rows, fields)=>{
        ctx.response.body = {
            ok: false,
            payload: {
                err: err,
                rows: rows,
                fields: fields
            }
        }
    })
}

module.exports = {
    'POST /api/execsql': execsql
}