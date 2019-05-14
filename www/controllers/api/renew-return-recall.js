// 'use strict'

const UserPriv = require('./internal/userprivvalidate')
const utils = require('../../misc/utils')
function CheckUser(ctx) {
    if(!UserPriv.loggedIn(ctx)) {
        ctx.response.body = {
            ok: false,
            msg: 'Usr not logged in'
        }
        return false
    }
    return true
}

function SQLError(ctx, err) {
    if(err && err.retcode != 0) {
        console.log(err)
        ctx.response.body = {
            ok: false,
            msg: 'SQL Error'
        }
        return true
    }
    return false
}

function RecordExist(ctx, rows) {
    if(rows.length === 0) {
        ctx.response.body = {
            ok: false,
            msg: `Can't Find Record`
        }
        return false
    }
    return true
}


function returnBook(ctx, next)
{
    if(!CheckUser(ctx)) return

    let isbn = ctx.request.body.isbn
    let copy_id = ctx.request.body.copy_id
    let usr_name = ctx.session.usr_name
    
    conn.query(`select * from borrow where ISBN='${isbn}' and copy_id=${copy_id} and usr_name='${usr_name}'`, (err, rows)=>{
        if(SQLError(ctx, err)) return

        if(!RecordExist(ctx, rows))  return

        let failed = true
        conn.query(`delete from borrow where ISBN='${isbn}' and copy_id=${copy_id} and usr_name='${usr_name}'`, (err, rows)=>{
            if(SQLError(ctx, err)) return
            failed = false;
        })

        if(failed) return
        failed = true
        let tick = utils.time_to_str(rows[0].borrow_time)
        let tock = utils.time_to_str(utils.get_timestamp())
        let num_renews = rows[0].num_renews
        conn.query(`insert into stale_borrow values ('${isbn}', ${copy_id}, '${usr_name}', '${tick}', '${tock}', ${num_renews})`, (err, rows)=>{
            if(SQLError(ctx, err)) return

            failed = false;
        })

        if(failed) return
        failed = true

        conn.query(`update copy set stat = 'in' where ISBN='${isbn}' and copy_id=${copy_id}`, (err)=>{
            if(SQLError(ctx, err)) return

            failed = false;
        })

        if(failed) return


        ctx.response.body = {
            ok: true,
        }
        return
    })
}  

function renewBook(ctx, next) {
    if(!CheckUser(ctx)) return

    let isbn = ctx.request.body.isbn
    let copy_id = ctx.request.body.copy_id
    let usr_name = ctx.session.usr_name
    let max_renew_allowed = ctx.session.max_renew_allowed

    let failed = true
    let condition = `ISBN='${isbn}' and copy_id=${copy_id} and usr_name='${usr_name}'`
    var old_date = {}
    conn.query(`select * from borrow where ${condition}`,
    (err, rows)=>{
        if(SQLError(ctx, err)) return
        if(!RecordExist(err, rows)) return

        if(max_renew_allowed >= 0 && rows[0].num_renews >= max_renew_allowed) {
            ctx.response.body = {
                ok: false,
                msg: `Renew Request Exceeds Allowance, Pls Return Book First`
            }
            return
        }

        old_date = rows[0].return_time
        failed = false
    })

    if(failed) return
    failed = true
    // console.log(old_date)
{
    let time_gap = utils.get_timestamp() 
    if(utils.time_cmp(old_date, time_gap) < 0) {
        ctx.response.body = {
            ok: false,
            msg: `Book is overdue, return first`
        }
        return
    }
    time_gap = utils.sub_time(old_date, time_gap) 
    
    // console.log('time_gap vs min_tim', time_gap, config.min_time_gap_renew, utils.time_cmp(time_gap, config.min_time_gap_renew))
    if(utils.time_cmp(time_gap, config.min_time_gap_renew) > 0) {
        time_gap = utils.sub_time(old_date, config.min_time_gap_renew)
        ctx.response.body = {
            ok: false,
            msg: `Pls renew after ${time_gap.year}-${time_gap.month}-${time_gap.date}`
        }
        return
    }
}
     
    
    conn.query(`select * from recall where ISBN='${isbn}'`, (err, rows)=>{
        if(SQLError(ctx, err)) return

        if(rows.length) {
            ctx.response.body = {
                ok:false,
                msg: 'Book has been RECALLED'
            }
            return
        }

        failed = false
    })

    if(failed) return
    failed = true
    console.log(old_date, config.renew_time)
    var new_date = utils.add_timestamp(old_date, config.renew_time)
    var new_date_str = utils.time_to_str(new_date)
    conn.query(`update borrow set num_renews = num_renews + 1 , return_time='${new_date_str}' where ${condition}`, (err)=>{
        if(SQLError(ctx, err)) return
        failed = false
    })

    if(failed) return

    ctx.response.body = {
        ok: true,
        payload: {
            new_date: new_date
        }
    }
}

function recallBook(ctx, next) {
    if(!CheckUser(ctx)) return

    let isbn = ctx.request.body.isbn
    // let copy_id = ctx.request.body.copy_id
    let usr_name = ctx.session.usr_name

    let failed = true
    conn.query(`select count(*) from copy where ISBN='${isbn}' and stat='in'`, (err, rows, fields)=>{
        if(SQLError(ctx, err)) return

        if(rows[0][fields[0].columnname] !== 0) {
            ctx.response.body = {
                ok:false,
                msg: 'There are other Copies In Circulation'
            }
            return
        }
        failed = false
    })

    if(failed) return
    failed = true
/*
    conn.query(`select count(*) from copy where ISBN='${isbn}' and stat='out'`, (err, rows, fields)=>{
        if(SQLError(ctx, err)) return

        if(rows[0][fields[0].columnname] !== 0) {
            ctx.response.body = {
                ok:false,
                msg: 'All books are Not for circulation'
            }
            return
        }
        failed = false
    })*/

    if(failed) return
    failed = true

    conn.query(`select count(*) from recall where usr_name = '${usr_name}' and ISBN '${ISBN}')`, (err, rows, fields)=>{
        if(SQLError(ctx, err)) return

        if(rows[0][fields[0].columnname] !== 0) {
            ctx.response.body = {
                ok:false,
                msg: 'You Have Already Recalled the Book'
            }
            return
        }
        failed = false
    })

    if(failed) return
    failed = true

    conn.query(`insert into recall values ('${usr_name}', '${ISBN}')`, (err, rows)=>{
        if(SQLError(ctx, err)) return
        failed = false
    })

    if(failed) return

    ctx.response.body = {
        ok: true
    }
}

module.exports = {
    'POST /api/returnbook': returnBook,
    'POST /api/renewbook': renewBook,
    'POST /api/recallbook': recallBook
}