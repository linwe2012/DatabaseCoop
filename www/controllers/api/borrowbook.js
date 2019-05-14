const UserPriv = require('./internal/userprivvalidate')
const utils = require('../../misc/utils')
function BorrowBook(ctx, next) {
    let isbn = ctx.request.body.isbn
    let copy_id = ctx.request.body.copy_id
    let usr_name = ctx.session.usr_name
    ctx.response.type = 'json'
    
    if(!isbn || !copy_id) {
        ctx.response.body = {
            ok:false,
            msg: 'Expected isbn and copy id'
        }
        return
    }

    copy_id = parseInt(copy_id)
    if(isNaN(copy_id)) {
        ctx.response.body = {
            ok:false,
            msg: 'Copy id is not number'
        }
        return
    }

    if(!UserPriv.loggedIn(ctx, next)) {
        console.log('log failed')
        ctx.response.body = {
            ok:false,
            msg: 'User not logged in'
        }
        return
    }
    console.log(copy_id)
    var failed = true

    conn.query(`select * from copy where ISBN='${isbn}' and copy_id=${copy_id}`, (err, rows)=>{
        if(err && err.retcode != 0) {
            console.log(err)
            ctx.response.body = {
                ok:false,
                msg: 'SQL Query Error#4'
            }
            return
        }

        if(rows.length == 0) {
            ctx.response.body = {
                ok:false,
                msg: 'No Such Book Copy'
            }
            return
        }

        if(rows[0].stat != 'in') {
            ctx.response.body = {
                ok:false,
                msg: 'This Book is NOT in circulation'
            }
            return
        }

        failed = false;
    })

    if(failed) return;

    if(ctx.session.max_num_book_allowed >= 0) { 
        failed = true

        conn.query(`select count(*) from borrow where usr_name='${usr_name}'`, (err, rows, fields)=>{
            if(err && err.retcode != 0) {
                console.log(err)
                ctx.response.body = {
                    ok:false,
                    msg: 'SQL Query Error#5'
                }
                return
            }

            console.log(rows)
            let borrowed = rows[0][fields[0].columnname]
            if(borrowed >= ctx.session.max_num_book_allowed) {
                ctx.response.body = {
                    ok:false,
                    msg: `Please first return books you've borrowed`
                }
                return
            }
            failed = false
        })
    }

    if(failed) return;
     // Now we offcially can borrow books after all the checks
    failed = true

    conn.query(`delete from recall where usr_name='${usr_name}' and ISBN='${isbn}'`, (err)=>{ if(err && err.retcode != 0) console.log(err)})
    conn.query(`update copy set stat='out' where ISBN='${isbn}' and copy_id=${copy_id}`, (err, rows)=>{
        if(err && err.retcode != 0) {
            console.log(err)
            ctx.response.body = {
                ok:false,
                msg: 'SQL Query Error#6'
            }
           return
        }
        failed = false
    })
    
   
    

    if(failed) return;

    failed = true

    var timestamp = utils.get_timestamp()
    let return_time = utils.add_timestamp(
        config.borrow_time, timestamp
    )
    console.log(timestamp, return_time)
    let start = utils.time_to_str(timestamp)
    let end = utils.time_to_str(return_time)
   let sql = `insert into borrow values('${isbn}', ${copy_id}, '${usr_name}', '${start}', '${end}', 0)`;
    conn.query(sql, (err)=>{
            console.log(sqlhandles.insert_borrow)
            if(err && err.retcode != 0) {
                console.log(err)
                ctx.response.body = {
                    ok:false,
                    msg: 'SQL Query Error#7'
                }
                return
            }
            failed = false
        })
    if(failed) {
        return
    }
    ctx.response.body = {
        ok:true
    }
}

module.exports = {
    'POST /api/borrowbook': BorrowBook
}