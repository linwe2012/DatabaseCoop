const fs = require('fs')
const UserPriv = require('./api/internal/userprivvalidate')

var fn_fetchAll = async (ctx, next)=>{
    if(!UserPriv.isAdmin(ctx, next)) {
        ctx.response.status = httpstatus.forbidden
        return
    }
    var tables = []
    conn.query('show tables', (err, rows, fields)=>{
        for(row of rows) {
            let table_name = row[fields[0].columnname]
            conn.query(`select * from ${table_name}`, (err, rows, fields)=>{
                tables.push({title: table_name, rows:rows, fields:fields})
            })
        }
    })
    ctx.render('dashboard.html', {
        avatar : '/static/img/avatar.png',
        tables : tables,
        types: sqltypes
    })
}

module.exports = {
    'GET /dashboard': fn_fetchAll
}