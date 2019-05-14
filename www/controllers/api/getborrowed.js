const UserPriv = require('./internal/userprivvalidate')
const utils = require('../../misc/utils')
function getBorrowed(ctx, next) {
    if(!UserPriv.loggedIn(ctx, next)) {
        ctx.response.body = {
            ok:false,
            msg: 'Not Logged In'
        }
        return
    }
    usr_name  = ctx.session.usr_name;
    let failed = true
    var res = {}
    let current = utils.get_timestamp()
    conn.query(`select * from borrow where usr_name = '${usr_name}'`, function(err, rows){
        if(err && err.retcode != 0) {
            console.log(err)
            ctx.response.body = {
                ok:false,
                msg: 'SQL Error'
            }
            return
        }
        var i = 0
        for(let r of rows) {
            if(utils.time_cmp(r.return_time, current) <= 0) {
                rows[i]['status'] = 'overdue'
            }
            else {
                let warning_min = utils.sub_time(r.return_time, config.returnbook_warning)
                //console.log(warning_min, '=', r.return_time, '-', config.returnbook_warning)
                //console.log(utils.time_cmp(warning_min, current))
                if(utils.time_cmp(warning_min, current) <= 0 ) {
                    rows[i]['status'] = 'warning'
                }
                else {
                    rows[i]['status'] = 'ok'
                }
            }
            ++i
        }
        res['borrow'] = rows
        failed  = false
    });   

    if(failed) return;
    failed = true;

    conn.query(`select * from stale_borrow where usr_name = '${usr_name}'`, function(err, rows){
        if(err && err.retcode != 0) {
            console.log(err)
            ctx.response.body = {
                ok:false,
                msg: 'SQL Error'
            }
            return
        }
        res['stale_borrow'] = rows
        failed  = false
    });

    if(failed) return;
    failed = true;

    ctx.response.body = {
        ok: true,
        payload: res
    }
}

module.exports = {
    'POST /api/getborrowed': getBorrowed
}